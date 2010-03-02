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

#ifndef _FLM_CORE_PRIVATE_FILTER_H_
# define _FLM_CORE_PRIVATE_FILTER_H_

#include <sys/queue.h>

#include <stdint.h>
#include <unistd.h>

#include "flm/core/public/buffer.h"
#include "flm/core/public/filter.h"
#include "flm/core/public/file.h"

#include "flm/core/private/obj.h"

#define FLM__TYPE_FILTER	0x000E0000

typedef int (*flm__FilterPerf_f)					\
(flm_Filter * filter, flm_Filter * in, off_t off, size_t count);

struct flm__FilterInput
{
	union {
		flm_Obj *		obj;
		flm_File *		file;
		flm_Buffer *		buffer;
	} class;
	enum {
		FLM__FILTER_TYPE_FILE,
		FLM__FILTER_TYPE_BUFFER
	} type;

	off_t				off;
	size_t				count;
	int				tried;
	TAILQ_ENTRY (flm__FilterInput)	entries;
};

struct flm_Filter
{
	/* inheritance */
	struct flm_Obj				obj;

	TAILQ_HEAD (queue, flm__FilterInput)	inputs;

	/* IO methods */
	struct {
		flm__FilterPerf_f			pipe;
	} perf;
};

int
flm__FilterInit (flm_Filter * filter);

void
flm__FilterPerfDestruct (flm_Filter * filter);

#endif /* !_FLM_CORE_PRIVATE_FILTER_H_ */
