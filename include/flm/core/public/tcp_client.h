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

#ifndef _FLM_CORE_PUBLIC_TCP_CLIENT_H_
# define _FLM_CORE_PUBLIC_TCP_CLIENT_H_

#include <stdint.h>

#include "flm/core/public/buffer.h"
#include "flm/core/public/container.h"

typedef struct flm_TCPClient flm_TCPClient;

#include "flm/core/public/monitor.h"
#include "flm/core/public/obj.h"

#define FLM_TCP_CLIENT(_obj) FLM_CAST(_obj, flm_TCPClient)

typedef void (*flm_TCPClientConnectHandler)	\
(void * state, int fd);

/**
 * \brief Create a new TCPClient object and connect it to \c host:port.
 *
 * Note that the connection will be handled in the background, thus a connection
 * error is more likely to appear in the error handler.
 *
 * \param monitor A pointer to the Monitor object used to monitor socket
 * availability.
 * \param cn_handler Handler called on connect.
 * \param rd_handler Handler called on read.
 * \param wr_handler Handler called on write.
 * \param cl_handler Handler called on close.
 * \param er_handler Handler called on error.
 * \param data User defined data.
 * \param host The hostname to connect to.
 * \param port The port to connect to.
 * \return A new TCPClient object.
 */
flm_TCPClient *
flm_TCPClientNew (flm_Monitor *				monitor,
		  const char *				host,
		  uint16_t				port,
		  void *				state);

#endif /* !_FLM_CORE_PUBLIC_TCP_CLIENT_H_ */
