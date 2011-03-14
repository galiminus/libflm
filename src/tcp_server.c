/*
 * Copyright (c) 2010-2011, Victor Goya <phorque@libflm.me>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <netdb.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "flm/core/public/error.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/error.h"
#include "flm/core/private/tcp_server.h"

flm_TCPServer *
flm_TCPServerNew (flm_Monitor *		monitor,
		  const char *		interface,
		  uint16_t		port,
		  void *	state)
{
    flm_TCPServer * tcp_server;

    tcp_server = flm__Alloc (sizeof (flm_TCPServer));
    if (tcp_server == NULL) {
        return (NULL);
    }
    if (flm__TCPServerInit (tcp_server, monitor, interface, port, state) == -1) {
        flm__Free (tcp_server);
        return (NULL);
    }
    return (tcp_server);
}

void
flm_TCPServerOnAccept (flm_TCPServer *			tcp_server,
		       flm_TCPServerAcceptHandler	handler)
{
    tcp_server->ac.handler = handler;
    return ;
}

int
flm__TCPServerInit (flm_TCPServer *	tcp_server,
		    flm_Monitor *	monitor,
		    const char *	interface,
		    uint16_t		port,
		    void *	state)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char str_port[6];
    int fd;
    int error;
    long flags;

    (void) interface;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if (snprintf (str_port, sizeof (str_port), "%d", port) < 0) {
        goto error;
    }
    if ((error = getaddrinfo (NULL, str_port, &hints, &result)) != 0) {
        goto error;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((fd = socket(rp->ai_family,
                         rp->ai_socktype,
                         rp->ai_protocol)) == -1) {
            continue;
        }
        if (bind (fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close (fd);
    }
    if (rp == NULL) {
        goto error;
    }

    if ((flags = fcntl (fd, F_GETFL, NULL)) < 0) {
        flm__Error = FLM_ERR_ERRNO;
        goto close_fd;
    }
    if (fcntl (fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        goto close_fd;
    }

    if (setsockopt (fd,				\
                    SOL_SOCKET,			\
                    SO_REUSEADDR,			\
                    (int[]){1},			\
                    sizeof (int)) == -1) {
        goto close_fd;
    }

    if (listen (fd, 1024) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        goto close_fd;
    }

    if (flm__IOInit ((flm_IO *) tcp_server,         \
                     monitor,			\
                     fd,				\
                     state) == -1) {
        goto close_fd;
    }
    ((flm_Obj *)(tcp_server))->type = FLM__TYPE_TCP_SERVER;

    ((flm_IO *)(tcp_server))->perf.read	=       \
        (flm__IOSysRead_f) flm__TCPServerPerfRead;

    flm_TCPServerOnAccept (tcp_server, NULL);

    return (0);

  close_fd:
    close (fd);
  error:
    return (-1);
}

int
flm__TCPServerSysAccept (flm_TCPServer * tcp_server)
{
    struct sockaddr_in addr_in;
    struct sockaddr * addr;
    unsigned int addr_len;
    int fd;

    memset (&addr_in, 0, sizeof (struct sockaddr_in));
    addr_len = sizeof (struct sockaddr_in);
    addr = (struct sockaddr *) &addr_in;

#if defined(HAVE_ACCEPT4)
    if ((fd = accept4 (((flm_IO *)(tcp_server))->sys.fd,
                       addr,
                       &addr_len,
                       SOCK_NONBLOCK)) < 0) {
        flm__Error = FLM_ERR_ERRNO;
        goto error;
    }
#else
    long flags;

    if ((fd = accept (((flm_IO *)(tcp_server))->sys.fd,
                      addr,
                      &addr_len)) < 0) {
        flm__Error = FLM_ERR_ERRNO;
        goto error;
    }
    if ((flags = fcntl (fd, F_GETFL, NULL)) < 0) {
        flm__Error = FLM_ERR_ERRNO;
        goto close_fd;
    }
    if (fcntl (fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        flm__Error = FLM_ERR_ERRNO;
        goto close_fd;
    }
#endif
    return (fd);

  close_fd:
    close (fd);
  error:
    return (-1);
}

int
flm__TCPServerPerfRead (flm_TCPServer *	tcp_server,
			flm_Monitor *	monitor,
			uint8_t		count)
{
    int fd;

    (void) count;
    (void) monitor;

    if ((fd = flm__TCPServerSysAccept (tcp_server)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            ((flm_IO *)(tcp_server))->rd.can = 0;
        }
        else if (errno == EINTR) {
            ((flm_IO *)(tcp_server))->rd.can = 1;
        }
        else {
            ((flm_IO *)(tcp_server))->rd.can = 0;
            /**
             * Call the error handler
             */
            if (((flm_IO *)(tcp_server))->er.handler) {
                ((flm_IO *)(tcp_server))->er.handler ((flm_IO *) tcp_server,
                                                      ((flm_IO *)(tcp_server))->state,
                                                      flm_Error());
            }
            return (-1);
        }
    }
    else {
        ((flm_IO *)(tcp_server))->rd.can = 1;
        if (tcp_server->ac.handler) {
            tcp_server->ac.handler (tcp_server,
                                    ((flm_IO *)(tcp_server))->state,
                                    fd);
        }
    }
    return (0);
}
