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

#ifndef _FLM_CORE_PUBLIC_THREAD_H_
# define _FLM_CORE_PUBLIC_THREAD_H_

#ifndef _FLM__SKIP

typedef struct flm_Thread flm_Thread;

#include "flm/core/public/obj.h"
#include "flm/core/public/monitor.h"

#endif /* !_FLM__SKIP */

typedef void (*flm_ThreadCallHandler)	\
(flm_Thread * thread, void * state, void * params);

flm_Thread *
flm_ThreadNew (flm_Monitor *	monitor,
	       void *           state);

int
flm_ThreadJoin (flm_Thread *	thread);

int
flm_ThreadCall (flm_Thread *		thread,
		flm_ThreadCallHandler	handler,
		void *                  params);

flm_Thread *
flm_ThreadRetain (flm_Thread * thread);

void
flm_ThreadRelease (flm_Thread * thread);

#endif /* !_FLM_CORE_PUBLIC_THREAD_H_ */
