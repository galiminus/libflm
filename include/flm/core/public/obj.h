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

#ifndef _FLM_CORE_PUBLIC_OBJ_H_
# define _FLM_CORE_PUBLIC_OBJ_H_

typedef struct flm_Obj flm_Obj;

#define FLM_CAST(_obj,_type) ((_type *)_obj)
#define FLM_OBJ(_obj) FLM_CAST(_obj, flm_Obj)

enum flm_ObjError
{
	/**
	 * Object is not shared
	 */
	FLM_ERR_NOSHR	=	0x00010000,

	/**
	 * not implemented
	 */
	FLM_ERR_NOIMP
};

#endif /* !_FLM_CORE_PUBLIC_OBJ_H_ */