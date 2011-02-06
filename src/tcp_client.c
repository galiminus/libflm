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

#include "flm/core/public/container.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/error.h"
#include "flm/core/private/stream.h"
#include "flm/core/private/tcp_addr.h"
#include "flm/core/private/tcp_client.h"

flm_TCPClient *
flm_TCPClientNew (flm_Monitor *		monitor,
		  const char *		host,
		  uint16_t		port,
		  void *	state)
{
	flm_TCPClient * tcp_client;

	tcp_client = flm__Alloc (sizeof (flm_TCPClient));
	if (tcp_client == NULL) {
		return (NULL);
	}
	if (flm__TCPClientInit (tcp_client, monitor, host, port, state) == -1) {
		flm__Free (tcp_client);
		return (NULL);
	}
	return (tcp_client);
}

int
flm__TCPClientInit (flm_TCPClient *	tcp_client,
		    flm_Monitor *	monitor,
		    const char *	host,
		    uint16_t		port,
		    void *	state)
{
	int fd;
	flm_TCPAddr * tcp_addr;

	if ((fd = flm__IOSocket (AF_INET, SOCK_STREAM)) == -1) {
		goto error;
	}

	/* do it synchronously for now, the Addr class should run
	   asynchronously in the near future */
	if ((tcp_addr = flm_TCPAddrNew (host, port)) == NULL) {
		goto close_fd;
	}

	if (connect (fd,						\
		     FLM_ADDR (tcp_addr)->info->ai_addr,		\
		     FLM_ADDR (tcp_addr)->info->ai_addrlen) == -1 &&	\
	    errno != EINPROGRESS) {
		goto release_addr;
	}

	if (flm__StreamInit (FLM_STREAM (tcp_client), monitor, fd, state) == -1) {
		goto close_fd;
	}
	FLM_OBJ (tcp_client)->type = FLM__TYPE_TCP_CLIENT;

	FLM_IO (tcp_client)->perf.write =			\
		(flm__IOSysRead_f) flm__TCPClientPerfWrite;

	if (errno == EINPROGRESS) {
		tcp_client->connected = false;
		FLM_IO (tcp_client)->wr.want = true;
	}
	else {
		tcp_client->connected = true;
		FLM_IO_EVENT_WITH (tcp_client, cn, FLM_IO (tcp_client)->sys.fd);
	}

	return (0);

release_addr:
	flm_Release (FLM_OBJ (tcp_addr));
close_fd:
	close (fd);
error:
	return (-1);
}

void
flm__TCPClientPerfWrite (flm_TCPClient *	tcp_client,
			 flm_Monitor *		monitor,
			 uint8_t		count)
{
	int error;
	socklen_t len;

	/* the socket is in nonblocking mode, we need to check for
	 connection completion on writing */
	if (tcp_client->connected) {
		flm__StreamPerfWrite (FLM_STREAM (tcp_client), monitor, count);
		return ;
	}
	len = sizeof (error);
	if (getsockopt (FLM_IO (tcp_client)->sys.fd,		\
			SOL_SOCKET,				\
			SO_ERROR,				\
			&error,					\
			&len) == -1) {
		goto error;
	}
	if (error != 0 && error != EINPROGRESS) {
		goto error;
	}
	FLM_IO_EVENT_WITH (tcp_client, cn, FLM_IO (tcp_client)->sys.fd);
	tcp_client->connected = true;
	return ;

error:
	/* fatal error */
	flm__Error = error;
	flm_IOClose (FLM_IO (tcp_client));
	FLM_IO_EVENT_WITH (FLM_IO (tcp_client), er, error);
	return ;
}
