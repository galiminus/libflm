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

#include "flm/core/private/addr.h"
#include "flm/core/private/alloc.h"

int
flm__AddrInit (flm_Addr *		addr,
	       const char *		node,
	       const char *		service,
	       const struct addrinfo *	hints)
{
	int error;

	if (flm__ObjInit (FLM_OBJ (addr)) == -1) {
		goto error;
	}
	FLM_OBJ (addr)->type = FLM__TYPE_ADDR;

	FLM_OBJ (addr)->perf.destruct =				\
		(flm__ObjPerfDestruct_f) flm__AddrPerfDestruct;

	if ((error = getaddrinfo (node, service, hints, &addr->info)) != 0) {
		goto object_destruct;
	}
	return (0);

object_destruct:
error:
	return (-1);
}

void
flm__AddrDestruct (flm_Addr * addr)
{
	freeaddrinfo (addr->info);
	return ;
}
