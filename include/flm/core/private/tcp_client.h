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

#ifndef _FLM_CORE_PRIVATE_TCP_CLIENT_H_
# define _FLM_CORE_PRIVATE_TCP_CLIENT_H_

#include <stdbool.h>

#include "flm/core/public/tcp_client.h"

#include "flm/core/private/io.h"
#include "flm/core/private/monitor.h"

#define FLM__TYPE_TCP_CLIENT	0x00150000

struct flm_TCPClient
{
	/* inheritance */
	struct flm_IO				io;

	/* members */
	bool					connected;

	struct {
		flm_TCPClientConnectHandler	handler;
	} cn;
};

int
flm__TCPClientInit (flm_TCPClient *			tcp_client,
		    flm_Monitor *			monitor,
		    flm_TCPClientConnectHandler		cn_handler,
		    flm_TCPClientReadHandler		rd_handler,
		    flm_TCPClientWriteHandler		wr_handler,
		    flm_TCPClientCloseHandler		cl_handler,
		    flm_TCPClientErrorHandler		er_handler,
		    void *				data,
		    const char *			host,
		    uint16_t				port);

void
flm__TCPClientPerfWrite (flm_TCPClient *	tcp_client,
			 flm_Monitor *		monitor,
			 uint8_t		count);

#endif /* !_FLM_CORE_PRIVATE_TCP_CLIENT_H_ */
