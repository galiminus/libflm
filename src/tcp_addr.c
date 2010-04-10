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

#include <stdint.h>

#include "flm/core/private/tcp_addr.h"
#include "flm/core/private/alloc.h"

flm_TCPAddr *
flm_TCPAddrNew (const char *		hostname,
		uint16_t		port)
{
	flm_TCPAddr * TCPAddr;

	if ((TCPAddr = flm_SlabAlloc (sizeof (flm_TCPAddr))) == NULL) {
		return (NULL);
	}
	if (flm__TCPAddrInit (TCPAddr, hostname, port) == -1) {
		flm_SlabFree (TCPAddr);
		return (NULL);
	}
	return (TCPAddr);
}

int
flm__TCPAddrInit (flm_TCPAddr *		tcpaddr,
		  const char *		hostname,
		  uint16_t		port)
{
	if (flm__ObjInit (FLM_OBJ (tcpaddr)) == -1) {
		return (-1);
	}
	FLM_OBJ (tcpaddr)->type = FLM__TYPE_TCP_ADDR;

	return (0);
}
