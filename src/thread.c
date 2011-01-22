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

#include <stdbool.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/thread.h"

flm_Thread *
flm_ThreadNew (flm_Container * state)
{
	flm_Thread * thread;

	if ((thread = flm__Alloc (sizeof (flm_Thread))) == NULL) {
		return (NULL);
	}
	if (flm__ThreadInit (thread, state) == -1) {
		flm__Free (thread);
		return (NULL);
	}
	return (thread);
}

int
flm__ThreadInit (flm_Thread *		thread,
		 flm_Container *	state)
{
	int				error;

	if (flm__ObjInit (FLM_OBJ (thread)) == -1) {
		goto error;
	}
	FLM_OBJ (thread)->type = FLM__TYPE_THREAD;

	thread->state = flm_Retain (FLM_OBJ (state));

	TAILQ_INIT (&(thread->msgs));
	
	error = pthread_cond_init (&(thread->cond), NULL);
	if (error != 0) {
		goto error;
	}
	
	error = pthread_mutex_init (&(thread->lock), NULL);
	if (error != 0) {
		goto destroy_cond;
	}
	
	error = pthread_create (&(thread->pthread),
				NULL,
				flm__ThreadStartRoutine,
				thread);
	if (error != 0) {
		goto destroy_lock;
	}
	return (0);

destroy_lock:
	pthread_mutex_destroy(&(thread->lock));
destroy_cond:
	pthread_cond_destroy(&(thread->cond));
error:
	return (-1);
}

void *
flm__ThreadStartRoutine (void *	_thread)
{
	flm_Thread *		thread = _thread;
	struct flm__Msg *	msg;
	struct flm__Msg		temp;
	int			error;

	error = pthread_mutex_lock (&(thread->lock));
	if (error != 0) {
		return ((void *)(error));
	}

	while (true) {
		TAILQ_FOREACH (msg, &(thread->msgs), entries) {
			if (msg->handler) {
				temp.entries = msg->entries;
				msg->handler(thread->state, msg->params);
				TAILQ_REMOVE (&(thread->msgs), msg, entries);
				flm__Free (msg);
				msg = &temp;
			}
		}
		error = pthread_cond_wait (&(thread->cond), &(thread->lock));
		if (error != 0) {
			return ((void *)(error));
		}
	}

	return ((void *)(0));
}

int
flm_ThreadJoin (flm_Thread * thread)
{
	int		error;

	error = pthread_join (thread->pthread, NULL);
	if (error != 0) {
		return (-1);
	}
	return (0);
}

int
flm_ThreadCall (flm_Thread *		thread,
		flm_ThreadCallHandler	handler,
		flm_Container *		params)
{
	struct flm__Msg *	msg;
	int			error;

	if ((msg = flm__Alloc (sizeof (struct flm__Msg))) == NULL) {
		goto error;
	}
	msg->handler = handler;
	msg->params = flm_Retain (FLM_OBJ (params));

	printf ("LOCK ?\n");

	error = pthread_mutex_lock (&(thread->lock));
	if (error != 0) {
		goto free_msg;
	}

	TAILQ_INSERT_TAIL (&(thread->msgs), msg, entries);

	error = pthread_mutex_unlock (&(thread->lock));
	if (error != 0) {
		goto free_msg;
	}

	printf ("INSERTED\n");

       	error = pthread_cond_signal (&(thread->cond));
	if (error != 0) {
		goto error;
	}

	printf ("OUT\n");

	return (0);

free_msg:
	flm__Free (msg);
error:
	return (-1);
}
