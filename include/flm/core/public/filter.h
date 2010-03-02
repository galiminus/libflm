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

#ifndef _FLM_CORE_PUBLIC_FILTER_H_
# define _FLM_CORE_PUBLIC_FILTER_H_

typedef struct flm_Filter flm_Filter;

#include "flm/core/public/buffer.h"
#include "flm/core/public/file.h"

#define FLM_FILTER(_obj) FLM_CAST(_obj, flm_Filter)

int
flm_FilterAppendFile (flm_Filter * filter,
		      flm_File * file,
		      off_t off,
		      size_t count);

int
flm_FilterAppendBuffer (flm_Filter * filter,
			flm_Buffer * buffer,
			off_t off,
			size_t count);

#endif /* !_FLM_CORE_PUBLIC_FILTER_H_ */
