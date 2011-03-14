/*
 * Copyright (c) 2010-2011, Victor Goya <phorque@libflm.me>
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

#include "flm/core/private/alloc.h"
#include "flm/core/private/error.h"

#include "flm/core/private/obj.h"

const char * flm__ObjErrors[] =
{
    "No more memory avaible."
};

void
flm__ObjInit (flm_Obj * obj)
{
    /* basic type */
    obj->type = FLM_TYPE_OBJ;

    /* methods */
    obj->perf.retain = flm__PerfRetain;
    obj->perf.release = flm__PerfRelease;
    obj->perf.destruct = NULL;
    
    /* reference counter */
    obj->stat.refcount = 1;
    
    flm__ErrorAdd (FLM_TYPE_OBJ >> 16, flm__ObjErrors);
    
    return ;
}

void *
flm__Retain (flm_Obj * obj)
{
    if (obj->perf.retain) {
        return (obj->perf.retain (obj));
    }
    return (obj);
}

void
flm__Release (flm_Obj * obj)
{
    if (obj->perf.release) {
        return (obj->perf.release (obj));
    }
    return ;
}

void *
flm__PerfRetain (flm_Obj * obj)
{
	if (obj == NULL) {
		return (NULL);
	}
	obj->stat.refcount++;
	return (obj);
}

void
flm__PerfRelease (flm_Obj * obj)
{
	if (obj == NULL) {
		return ;
	}
	obj->stat.refcount--;
	if (obj->stat.refcount == 0) {
		if (obj->perf.destruct) {
			obj->perf.destruct (obj);
		}
		flm__Free (obj);
	}
	return ;
}
