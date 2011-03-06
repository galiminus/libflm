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

#ifndef _FLM_CORE_PRIVATE_BUFFER_H_
# define _FLM_CORE_PRIVATE_BUFFER_H_

#include <stdarg.h>
#include <stdbool.h>

#include "flm/core/public/buffer.h"

#include "flm/core/private/obj.h"

#define FLM__TYPE_BUFFER	0x00020000

#define FLM_BUFFER(_obj) FLM_CAST((_obj), flm_Buffer)

struct flm_Buffer
{
	/* inheritance */
	struct flm_Obj				obj;

	/* members */
	char *					content;
	size_t					len;

	struct {
		flm_BufferFreeContentHandler	handler;
	} fr;
};

void
flm__BufferInit (flm_Buffer *			buffer,			\
		 char *				content,		\
		 size_t				len,			\
		 flm_BufferFreeContentHandler	fr_handler);

void
flm__BufferPerfDestruct (flm_Buffer * buffer);

#endif /* !_FLM_CORE_PRIVATE_BUFFER_H_ */
