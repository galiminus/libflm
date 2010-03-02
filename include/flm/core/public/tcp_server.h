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

#ifndef _FLM_CORE_PUBLIC_TCP_SERVER_H_
# define _FLM_CORE_PUBLIC_TCP_SERVER_H_

#include <stdint.h>

typedef struct flm_TCPServer flm_TCPServer;

#include "flm/core/public/monitor.h"
#include "flm/core/public/obj.h"

#define FLM_TCP_SERVER(_obj) FLM_CAST(_obj, flm_TCPServer)

typedef void (*flm_TCPServerAcceptHandler)				\
(flm_TCPServer * tcp_server, flm_Monitor * monitor, void * data, int fd);

typedef void (*flm_TCPServerCloseHandler)				\
(flm_TCPServer * tcp_server, flm_Monitor * monitor, void * data);

typedef void (*flm_TCPServerErrorHandler)				\
(flm_TCPServer * tcp_server, flm_Monitor * monitor, void * data);

flm_TCPServer *
flm_TCPServerNew (flm_Monitor * monitor,
		  flm_TCPServerAcceptHandler		ac_handler,
		  flm_TCPServerCloseHandler		cl_handler,
		  flm_TCPServerErrorHandler		er_handler,
		  void *				data,
		  const char *				interface,
		  uint16_t				port);

#endif /* !_FLM_CORE_PUBLIC_TCP_SERVER_H_ */
