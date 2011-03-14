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

#ifndef _FLM_CORE_PRIVATE_THREAD_H_
# define _FLM_CORE_PRIVATE_THREAD_H_

#include <sys/queue.h>

#include <pthread.h>

#include "flm/core/public/io.h"
#include "flm/core/public/monitor.h"
#include "flm/core/public/thread.h"

#define FLM__TYPE_THREAD	0x00090000

struct flm__Msg {
	flm_ThreadCallHandler	handler;
	void *		params;
	TAILQ_ENTRY (flm__Msg)	entries;
};

struct flm_Thread
{
	/* inheritance */
	struct flm_Obj			obj;

	void *                          state;
	flm_Monitor *			monitor;

	struct {
            int                         in;
            flm_Stream *		out;
	} pipe;

	pthread_t			pthread;
	pthread_cond_t			cond;
	pthread_mutex_t			lock;
	TAILQ_HEAD (msg, flm__Msg)	msgs;
};

int
flm__ThreadInit (flm_Thread *		thread,
		 flm_Monitor *		monitor,
		 void *               state);

void
flm__ThreadPerfDestruct (flm_Thread * thread);

void
flm__ThreadExit (flm_Thread *   thread,
                 void *         _state,
                 void *         _params);

void
flm__ThreadEventHandler (flm_Stream *   pipe,
                         void *         _thread,
                         flm_Buffer *   _buffer);

void *
flm__ThreadStartRoutine (void *		_thread);

void
flm__ThreadPerfRelease (flm_Thread * thread);

#endif /* !_FLM_CORE_PRIVATE_THREAD_H_ */
