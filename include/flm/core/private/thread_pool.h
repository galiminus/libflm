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

#ifndef _FLM_CORE_PRIVATE_THREAD_POOL_H_
# define _FLM_CORE_PRIVATE_THREAD_POOL_H_

#include <sys/queue.h>

#include <pthread.h>

#include "flm/core/public/thread.h"
#include "flm/core/public/thread_pool.h"

#define FLM__TYPE_THREAD_POOL	0x00140000

struct flm_ThreadPool
{
	/* inheritance */
	struct flm_Obj		obj;

	pthread_mutex_t		lock;

	uint32_t		count;
	uint32_t		current;
	flm_Thread **		threads;
};

int
flm__ThreadPoolInit (flm_ThreadPool *		thread_pool);

void *
flm__ThreadStartRoutine (void *			_params);

#endif /* !_FLM_CORE_PRIVATE_THREAD_POOL_H_ */
