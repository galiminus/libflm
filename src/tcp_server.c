/*
 * Copyright (c) 2008-2009, Victor Goya <phorque@libflm.me>
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

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/tcp_server.h"

flm_TCPServer *
flm_TCPServerNew (flm_Monitor * monitor,
		  flm_TCPServerAcceptHandler		ac_handler,
		  flm_TCPServerCloseHandler		cl_handler,
		  flm_TCPServerErrorHandler		er_handler,
		  void *				data,
		  const char *				interface,
		  uint16_t				port)
{
	flm_TCPServer * tcp_server;

	tcp_server = flm_SlabAlloc (sizeof (flm_TCPServer));
	if (tcp_server == NULL) {
		return (NULL);
	}
	if (flm__TCPServerInit (tcp_server,				\
				monitor,				\
				ac_handler,				\
				cl_handler,				\
				er_handler,				\
				data,					\
				interface,				\
				port) == -1) {
		flm_SlabFree (tcp_server);
		return (NULL);
	}
	return (tcp_server);
}

int
flm__TCPServerInit (flm_TCPServer *			tcp_server,
		    flm_Monitor *			monitor,
		    flm_TCPServerAcceptHandler		ac_handler,
		    flm_TCPServerCloseHandler		cl_handler,
		    flm_TCPServerErrorHandler		er_handler,
		    void *				data,
		    const char *			interface,
		    uint16_t				port)
{
	if (flm__IOInitSocket (FLM_IO (tcp_server),			\
			       monitor,					\
			       (flm__IOCloseHandler) cl_handler,	\
			       (flm__IOErrorHandler) er_handler,	\
			       data,					\
			       AF_INET,					\
			       SOCK_STREAM) == -1) {
		goto error;
	}
	FLM_OBJ (tcp_server)->type = FLM__TYPE_TCP_SERVER;

	FLM_IO (tcp_server)->perf.read =				\
		(flm__IOSysRead_f) flm__TCPServerPerfRead;

	if (setsockopt (FLM_IO (tcp_server)->sys.fd,			\
			SOL_SOCKET,					\
			SO_REUSEADDR,					\
			(int[]){1},					\
			sizeof (int)) == -1) {
		goto io_destruct;
	}

	if (flm__IOBind (FLM_IO (tcp_server),				\
			 interface,					\
			 port,						\
			 SOCK_STREAM) == -1) {
		goto io_destruct;
	}

	if (flm__IOListen (FLM_IO (tcp_server)) == -1) {
		goto io_destruct;
	}

	tcp_server->ac.handler = ac_handler;

	return (0);

io_destruct:
	flm__IOPerfDestruct (FLM_IO (tcp_server));
error:
	return (-1);
}

int
flm__TCPServerPerfRead (flm_TCPServer * tcp_server,
			flm_Monitor * monitor,
			uint8_t count)
{
	int fd;

	(void) count;

	if ((fd = flm__IOAccept (FLM_IO (tcp_server))) == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			FLM_IO (tcp_server)->rd.can = 0;
		}
		else if (errno == EINTR) {
			FLM_IO (tcp_server)->rd.can = 1;
		}
		else {
			FLM_IO (tcp_server)->rd.can = 0;
			FLM_IO_EVENT (FLM_IO (tcp_server), er, monitor);
		}
	}
	else {
		FLM_IO (tcp_server)->rd.can = 1;
		FLM_IO_EVENT_WITH (tcp_server, ac, monitor, fd);
	}
	return (0);
}
