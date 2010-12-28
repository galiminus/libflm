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

#ifndef _FLM_CORE_PRIVATE_CONTAINER_H_
# define _FLM_CORE_PRIVATE_CONTAINER_H_

#include "flm/core/public/container.h"

#include "flm/core/private/obj.h"

#define FLM__TYPE_CONTAINER	0x00AA0000

struct flm_Container
{
	struct flm_Obj				obj;

	void *					content;
	struct {
		flm_ContainerFreeHandler	handler;
	} fr;
};

int
flm__ContainerInit (flm_Container * container,
		    void * content,
		    flm_ContainerFreeHandler free_handler);

void
flm__ContainerPerfDestruct (flm_Container * container);

#endif /* !_FLM_CORE_PRIVATE_CONTAINER_H_ */
