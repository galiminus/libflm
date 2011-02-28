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

#ifndef _FLM_CORE_PRIVATE_OBJ_H_
# define _FLM_CORE_PRIVATE_OBJ_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "flm/core/public/obj.h"

#define FLM__TYPE_OBJ	0x00010000

typedef void		(*flm__ObjPerfDestruct_f) (flm_Obj *);

struct flm_Obj
{
	int				type;

	struct {
		uint32_t		refcount;
	} stat;

	struct {
		flm__ObjPerfDestruct_f	destruct;
	} perf;
};

void
flm__ObjInit (flm_Obj * obj);

#endif /* !_FLM_CORE_PRIVATE_OBJ_H_ */
