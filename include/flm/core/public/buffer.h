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

/**
 * \file buffer.h
 * \c The buffer class is a container for chunks of memory.
 */

#ifndef _FLM_CORE_PUBLIC_BUFFER_H_
# define _FLM_CORE_PUBLIC_BUFFER_H_

#include <unistd.h>
#include <stdarg.h>

typedef struct flm_Buffer flm_Buffer;

typedef void (*flm_BufferFreeContentHandler)(void * ptr);

#include "flm/core/public/obj.h"

#define FLM_BUFFER(_obj) FLM_CAST(_obj, flm_Buffer)

flm_Buffer *
flm_BufferNew (char *				content,		\
	       size_t				len,			\
	       flm_BufferFreeContentHandler	fr_handler);

size_t
flm_BufferLength (flm_Buffer * buffer);

char *
flm_BufferContent (flm_Buffer * buffer);

#endif /* !_FLM_CORE_PUBLIC_BUFFER_H_ */
