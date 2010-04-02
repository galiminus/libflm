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

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/buffer.h"
#include "flm/core/private/error.h"
#include "flm/core/private/file.h"
#include "flm/core/private/io.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/monitor.h"

const char * flm__IOErrors[] =
{
	"Object is not shared",
	"Not implemented"
};

void
flm_IOShutdown (flm_IO *	io)
{
	io->rd.want = false;
	io->cl.shutdown = true;
	return ;
}

void
flm_IOClose (flm_IO *		io)
{
	io->rd.can = false;
	io->rd.want = false;

	io->wr.can = false;
	io->wr.want = false;

	io->cl.shutdown = true;

	return ;
}

int
flm__IOInit (flm_IO *			io,
	     flm_Monitor *		monitor,
	     flm__IOCloseHandler	cl_handler,
	     flm__IOErrorHandler	er_handler,
	     void *			data,
	     int			fd)
{
	if (flm__FilterInit (FLM_FILTER (io)) == -1) {
		goto error;
	}
	FLM_OBJ (io)->type = FLM__TYPE_IO;

	FLM_OBJ (io)->perf.destruct =					\
		(flm__ObjPerfDestruct_f) flm__IOPerfDestruct;

	io->sys.fd		=	fd;

	io->rd.can		=	false;
	io->rd.want		=	true;
	io->rd.limit		=	4;

	io->wr.can		=	false;
	io->wr.want		=	false;
	io->wr.limit		=	4;

	io->cl.shutdown		=	false;
	io->cl.handler		=	cl_handler;

	io->er.handler		=	er_handler;

	io->user.data		=	data;

	io->perf.read		=	NULL;
	io->perf.write		=	NULL;
	io->perf.close		=	flm__IOPerfClose;

	flm__ErrorAdd (FLM__TYPE_IO >> 16, flm__IOErrors);

	if (flm__MonitorIOAdd (monitor, io) == -1) {
		goto filter_destruct;
	}

	return (0);

filter_destruct:
	flm__FilterPerfDestruct (FLM_FILTER (io));
error:
	return (-1);
}

int
flm__IOInitSocket (flm_IO *		io,
		   flm_Monitor *	monitor,
		   flm__IOCloseHandler	cl_handler,
		   flm__IOErrorHandler	er_handler,
		   void *		data,
		   int			domain,
		   int			type)
{
	int fd;

	if ((fd = flm__IOSocket (domain, type)) == -1) {
		goto error;
	}
	if (flm__IOInit (io, monitor, cl_handler, er_handler, data, fd) != 0) {
		goto close_fd;
	}
	return (0);

close_fd:
	close (fd);
error:
	return (-1);
}

void
flm__IOPerfDestruct (flm_IO * io)
{
	int retry;

	/* ensure that a fd is really closed */
	retry = 0;
	while (close (io->sys.fd) == -1) {
		if (errno == EINTR) {
			continue ;
		}
		if (errno == EBADF) {
			break ; /* shouldn't happen */
		}
		if (retry == 3) { /* retry 2 times then give up */
			break ;
		}
		retry++;
	}
	flm__FilterPerfDestruct (FLM_FILTER (io));
	return ;
}

int
flm__IOSocket (int	domain,
	       int	type)
{
	int fd;
	long flags;

	if ((fd = socket (domain, type, 0)) < 0) {
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
	return (fd);

close_fd:
	close (fd);
error:
	return (-1);
}

int
flm__IOAccept (flm_IO * io)
{
	struct sockaddr_in addr_in;
	struct sockaddr * addr;
	unsigned int addr_len;
	int fd;

	memset (&addr_in, 0, sizeof (struct sockaddr_in));
	addr_len = sizeof (struct sockaddr_in);
	addr = (struct sockaddr *) &addr_in;

#if defined(linu)
	if ((fd = accept4 (io->sys.fd, addr, &addr_len, SOCK_NONBLOCK)) < 0) {
		flm__Error = FLM_ERR_ERRNO;
		goto error;
	}
	return (fd);

error:
	return (-1);
#else
	long flags;

	if ((fd = accept (io->sys.fd, addr, &addr_len)) < 0) {
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
	return (fd);

close_fd:
	close (fd);
error:
	return (-1);
#endif
}

int
flm__IOBind (flm_IO * io,
	     const char * interface,
	     int port,
	     int type)
{
	struct addrinfo hints;
	struct addrinfo * res;
	struct addrinfo * rp;
	char sport[6];
	int error;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;
	hints.ai_flags = AI_PASSIVE;

	snprintf (sport, 6, "%d", port);
	if ((error = getaddrinfo (interface, sport, &hints, &res)) != 0) {
		goto error;
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		if (bind (io->sys.fd, rp->ai_addr, rp->ai_addrlen) != 0) {
			continue ;
		}
		break ;
	}
	if (rp == NULL) {
		flm__Error = FLM_ERR_ERRNO;
		goto free_res;
	}

	freeaddrinfo (res);
	return (0);

free_res:
	freeaddrinfo (res);
error:
	return (-1);
}

int
flm__IOListen (flm_IO * io)
{
	if (listen (io->sys.fd, 1024) == -1) {
		flm__Error = FLM_ERR_ERRNO;
		return (-1);
	}
	return (0);
}

uint8_t
flm__IORead (flm_IO *		io,
	     flm_Monitor *	monitor)
{
	uint8_t count;

	for (count = 0; count < io->rd.limit; count++) {
		if (!io->rd.want) {
			break ;
		}
		if (io->perf.read) {
			io->perf.read (io, monitor, count + 1);
		}
		if (!io->rd.can) {
			break ;
		}
	}
	return (count);
}

uint8_t
flm__IOWrite (flm_IO *		io,
	      flm_Monitor *	monitor)
{
	uint8_t count;

	for (count = 0; count < io->wr.limit; count++) {
		if (!io->wr.want) {
			break ;
		}
		if (io->perf.write) {
			io->perf.write (io, monitor, count + 1);
		}
		if (!io->wr.can) {
			break ;
		}
	}
	return (count);
}

void
flm__IOClose (flm_IO *		io,
	      flm_Monitor *	monitor)
{
	if (io->perf.close) {
		io->perf.close (io, monitor);
	}
	return ;
}


void
flm__IOPerfClose (flm_IO *	io,
		  flm_Monitor *	monitor)
{
	FLM_IO_EVENT (io, cl, monitor);
	if (flm__MonitorIODelete (monitor, io) == -1) {
		FLM_IO_EVENT (io, er, monitor);
	}
	return ;
}
