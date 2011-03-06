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

#include <errno.h>
#include <unistd.h>

#include "flm/core/public/io.h"
#include "flm/core/public/monitor.h"
#include "flm/core/public/stream.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/thread.h"

flm_Thread *
flm_ThreadNew (flm_Monitor *	monitor,
               void *           state)
{
    flm_Thread *        thread;
    
    if ((thread = flm__Alloc (sizeof (flm_Thread))) == NULL) {
        return (NULL);
    }
        
    if (flm__ThreadInit (thread, monitor, state) == -1) {
        flm__Free (thread);
        return (NULL);
    }
    return (thread);
}

int
flm__ThreadInit (flm_Thread *		thread,
                 flm_Monitor *		monitor,
                 void *                 state)
{
    int         error;
    int         thread_pipe[2];

    flm__ObjInit ((flm_Obj *) thread);

    ((flm_Obj *)(thread))->type = FLM__TYPE_THREAD;

    ((flm_Obj *)(thread))->perf.destruct =                      \
        (flm__ObjPerfDestruct_f) flm__ThreadPerfDestruct;

    ((flm_Obj *)(thread))->perf.release =               \
        (flm__ObjPerfRelease_f) flm__ThreadPerfRelease;
    
    thread->state = state;
    
    TAILQ_INIT (&(thread->msgs));
    
    thread->monitor = flm_MonitorRetain (monitor);
    
    if (pipe (thread_pipe) == -1) {
        goto release_monitor;
    }
    
    thread->pipe.out = flm_StreamNew (thread->monitor,
                                      thread_pipe[0],
                                      thread);
    
    flm_StreamOnRead (thread->pipe.out, flm__ThreadEventHandler);
    flm_StreamRelease (thread->pipe.out);

    thread->pipe.in = thread_pipe[1];
    
    error = pthread_mutex_init (&(thread->lock), NULL);
    if (error != 0) {
        goto close_pipe;
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
  close_pipe:
    close (thread_pipe[1]);
  release_monitor:
    flm_MonitorRelease (thread->monitor);
    return (-1);
}

void
flm__ThreadPerfDestruct (flm_Thread * thread)
{
    flm_ThreadCall (thread, flm__ThreadExit, NULL);
}

void
flm__ThreadExit (flm_Thread *   thread,
                 void *         _state,
                 void *         _params)
{
    (void) _state;
    (void) _params;

    flm_StreamClose (thread->pipe.out);
    close (thread->pipe.in);
}

void
flm__ThreadEventHandler (flm_Stream *   _stream,
                         void *         _thread,
                         flm_Buffer *   buffer)
{
    flm_Thread *        thread;
    struct flm__Msg *	msg;
    struct flm__Msg     temp;
    int			error;

    (void) _stream;

    thread = (flm_Thread *)_thread;

    flm_BufferRelease (buffer);

    error = pthread_mutex_lock (&(thread->lock));
    if (error != 0) {
        return ;
    }
    
    /* instead of executing everything, replace me by an empty
       list and execute me later */
    TAILQ_FOREACH (msg, &(thread->msgs), entries) {
        if (msg->handler) {
            temp.entries = msg->entries;
            msg->handler(thread, thread->state, msg->params);
            TAILQ_REMOVE (&(thread->msgs), msg, entries);
            flm__Free (msg);
            msg = &temp;
        }
    }
    
    error = pthread_mutex_unlock (&(thread->lock));
    if (error != 0) {
        return ;
    }
    
    return ;
}


void *
flm__ThreadStartRoutine (void *	_thread)
{
    flm_Thread *        thread = _thread;

    flm_MonitorWait (thread->monitor);
    flm_MonitorRelease (thread->monitor);

    return ((void *)(0));
}

void
flm__ThreadPerfRelease (flm_Thread * thread)
{
    if (thread == NULL) {
        return ;
    }
    ((flm_Obj *)(thread))->stat.refcount--;
    if (((flm_Obj *)(thread))->stat.refcount == 0) {
        if (((flm_Obj *)(thread))->perf.destruct) {
            ((flm_Obj *)(thread))->perf.destruct ((flm_Obj *) thread);
        }
        flm_ThreadJoin (thread);
        flm__Free (thread);
    }
    return ;
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
		void *                  params)
{
    struct flm__Msg *	msg;
    int			error;

    /**
     * Refuse the call if the thread is not the only
     * owner of its monitor to avoid some race conditions
     */
    if (((flm_Obj *)(thread->monitor))->stat.refcount > 1) {
        return (-1);
    }

    if ((msg = flm__Alloc (sizeof (struct flm__Msg))) == NULL) {
        goto error;
    }
    msg->handler = handler;
    msg->params = params;
    
    error = pthread_mutex_lock (&(thread->lock));
    if (error != 0) {
        goto free_msg;
    }
    
    TAILQ_INSERT_TAIL (&(thread->msgs), msg, entries);
    
    error = pthread_mutex_unlock (&(thread->lock));
    if (error != 0) {
        goto free_msg;
    }
    
    while (write (thread->pipe.in, "0", 1) == -1) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
            continue ;
        }
        goto free_msg;
    }
    
    return (0);
    
  free_msg:
    flm__Free (msg);
  error:
    return (-1);
}

flm_Thread *
flm_ThreadRetain (flm_Thread * thread)
{
    return (flm__Retain ((flm_Obj *) thread));
}

void
flm_ThreadRelease (flm_Thread * thread)
{
    flm__Release ((flm_Obj *) thread);
}
