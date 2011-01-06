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

#include "flm/core/private/alloc.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/thread_pool.h"

struct flm__ThreadParams
{
	flm_ThreadPool *	thread_pool;
	struct flm__Thread *	self;
};

flm_ThreadPool *
flm_ThreadPoolNew (uint32_t count)
{
	flm_ThreadPool * thread_pool;

	if ((thread_pool = flm__Alloc (sizeof (flm_ThreadPool))) == NULL) {
		return (NULL);
	}
	if (flm__ThreadPoolInit (thread_pool, count) == -1) {
		flm__Free (thread_pool);
		return (NULL);
	}
	return (thread_pool);
}

int
flm__ThreadPoolInit (flm_ThreadPool *	thread_pool,
		     uint32_t		count)
{
	uint32_t			i;
	int				error;
	struct flm__ThreadParams *	params;

	if (flm__ObjInit (FLM_OBJ (thread_pool)) == -1) {
		goto error;
	}
	FLM_OBJ (thread_pool)->type = FLM__TYPE_THREAD_POOL;

	thread_pool->count = count;
	thread_pool->current = 0;

	thread_pool->threads =						\
		flm__Alloc (sizeof (struct flm__Thread) * count);

	if (thread_pool->threads == NULL) {
		goto error;
	}
	
	for (i = 0; i < count; i++) {
		params = malloc (sizeof (struct flm__ThreadParams));
		if (params == NULL) {
			goto kill_threads;
		}
		params->thread_pool = thread_pool;
		params->self = &(thread_pool->threads[i]);

		TAILQ_INIT (&(thread_pool->threads[i].msgs));

		error = pthread_mutex_init (&(thread_pool->threads[i].cond),
					    NULL);
		if (error != 0) {
			goto kill_threads;
		}

		error = pthread_mutex_init (&(thread_pool->threads[i].lock),
					    NULL);
		if (error != 0) {
			goto kill_threads;
		}

		error = pthread_create (&(thread_pool->threads[i].pthread),
					NULL,
					flm__ThreadStartRoutine,
					params);
		if (error != 0) {
			goto kill_threads;
		}
	}
	return (0);

kill_threads:
	/* do it */
error:
	return (-1);
}

void *
flm__ThreadStartRoutine (void * _params)
{
	struct flm__ThreadParams *	params = _params;
	struct flm__Msg *		msg;
	int				error;

	error = pthread_mutex_lock (&(params->self->lock));
	if (error != 0) {
		return ((void *)(error));
	}

	TAILQ_FOREACH (msg, &(params->self->msgs), entries) {
		if (msg->handler) {
			msg->handler(msg->params);
		}
	}

	error = pthread_mutex_unlock (&(params->self->lock));
	if (error != 0) {
		return ((void *)(error));
	}

	return ((void *)(0));
}

int
flm__ThreadPoolCall (struct flm__Thread *	thread,
		     flm_ThreadPoolCallHandler	handler,
		     flm_Container *		params)
{
	struct flm__Msg *	msg;

	if ((msg = flm__Alloc (sizeof (struct flm__Msg))) == NULL) {
		goto error;
	}
	msg->handler = handler;
	msg->params = flm_Retain (FLM_OBJ (params));

	if (pthread_mutex_lock (&(thread->lock)) == -1) {
		goto free_msg;
	}

	TAILQ_INSERT_TAIL (&(thread->msgs), msg, entries);

	if (pthread_mutex_unlock (&(thread->lock)) == -1) {
		goto free_msg;
	}
	return (0);

free_msg:
	flm__Free (msg);
error:
	return (-1);
}

int
flm_ThreadPoolJoin (flm_ThreadPool * thread_pool)
{
	uint32_t	i;
	int		error;

	for (i = 0; i < thread_pool->count; i++) {
		error = pthread_join (thread_pool->threads[i].pthread, NULL);
		if (error != 0) {
			return (-1);
		}
	}
	return (0);
}

int
flm_ThreadPoolCall (flm_ThreadPool *		thread_pool,
		    flm_ThreadPoolCallHandler	handler,
		    flm_Container *		params)
{
	struct flm__Thread *	thread;
	uint32_t		index;

	index = thread_pool->current % thread_pool->count;
	thread = &(thread_pool->threads[index]);
	thread_pool->current++;

	if (flm__ThreadPoolCall (thread, handler, params) == -1) {
		return (-1);
	}

	return (0);
}

int
flm_ThreadPoolBroadcast (flm_ThreadPool *		thread_pool,
			 flm_ThreadPoolCallHandler	handler,
			 flm_Container *		params)
{
	struct flm__Thread *	thread;
	uint32_t		index;

	for (index = 0; index < thread_pool->count; index++) {
		thread = &(thread_pool->threads[index]);
		if (flm__ThreadPoolCall (thread, handler, params) == -1) {
			return (-1);
		}
	}
	return (0);
}
