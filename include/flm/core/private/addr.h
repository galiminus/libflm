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

#ifndef _FLM_CORE_PRIVATE_ADDR_H_
# define _FLM_CORE_PRIVATE_ADDR_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "flm/core/private/obj.h"

#define FLM_ADDR(_obj) FLM_CAST(_obj, flm_Addr)

#define FLM__TYPE_ADDR	0x00170000

typedef struct flm_Addr
{
	/* inheritance */
	struct flm_Obj		obj;

	/* members */
	struct addrinfo	*	info;
} flm_Addr;

flm_Addr *
flm_AddrNew (const char *		node,
	     const char *		service,
	     const struct addrinfo *	hints);

int
flm__AddrInit (flm_Addr *		addr,
	       const char *		node,
	       const char *		service,
	       const struct addrinfo *	hints);

void
flm__AddrPerfDestruct (flm_Addr * addr);

#endif /* !_FLM_CORE_PRIVATE_ADDR_H_ */
