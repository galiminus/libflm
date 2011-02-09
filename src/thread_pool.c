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

#include <string.h>

#include "flm/core/public/thread.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/thread_pool.h"

flm_ThreadPool *
flm_ThreadPoolNew ()
{
    flm_ThreadPool * thread_pool;

    if ((thread_pool = flm__Alloc (sizeof (flm_ThreadPool))) == NULL) {
        return (NULL);
    }
    if (flm__ThreadPoolInit (thread_pool) == -1) {
        flm__Free (thread_pool);
        return (NULL);
    }
    return (thread_pool);
}

int
flm__ThreadPoolInit (flm_ThreadPool *   thread_pool)
{
    int         error;

    if (flm__ObjInit (FLM_OBJ (thread_pool)) == -1) {
        return (-1);
    }
    FLM_OBJ (thread_pool)->type = FLM__TYPE_THREAD_POOL;

    thread_pool->count = 0;
    thread_pool->current = 0;

    thread_pool->threads = NULL;

    error = pthread_mutex_init (&(thread_pool->lock), NULL);
    if (error != 0) {
        return (-1);
    }

    return (0);
}

int
flm_ThreadPoolAdd (flm_ThreadPool *     thread_pool,
                   flm_Thread *         thread)
{
    flm_Thread **   threads;
    int             error;
    uint32_t        count;

    error = pthread_mutex_lock (&(thread_pool->lock));
    if (error != 0) {
        goto error;
    }

    count = thread_pool->count + 1;
    threads = flm__Alloc (count * sizeof (flm_Thread *));
    if (threads == NULL) {
        goto unlock;
    }
    if (thread_pool->threads) {
        memcpy (threads,                                        \
                thread_pool->threads,                           \
                thread_pool->count * sizeof(flm_Thread *));
    }
    flm__Free(thread_pool->threads);

    threads[count - 1] = thread;
    thread_pool->threads = threads;
    thread_pool->count = count;

    error = pthread_mutex_unlock (&(thread_pool->lock));
    if (error != 0) {
        goto error;
    }

    return (0);

  unlock:
    pthread_mutex_unlock (&(thread_pool->lock));
  error:
    return (-1);
}

int
flm_ThreadPoolJoin (flm_ThreadPool *            thread_pool)
{
    uint32_t        i;

    for (i = 0; i < thread_pool->count; i++) {
        if (flm_ThreadJoin (thread_pool->threads[i]) == -1) {
            return (-1);
        }
    }
    return (0);
}

int
flm_ThreadPoolCall (flm_ThreadPool *            thread_pool,
                    flm_ThreadCallHandler       handler,
                    void *                      params)
{
    flm_Thread *    thread;
    uint32_t        index;

    if (thread_pool->count == 0) {
        return (0);
    }

    index = thread_pool->current % thread_pool->count;
    thread_pool->current++;

    if (flm_ThreadPoolCallTo (thread_pool, index, handler, params) == -1) {
        return (-1);
    }

    return (0);
}

int
flm_ThreadPoolCallTo (flm_ThreadPool *          thread_pool,
                      uint32_t                  to,
                      flm_ThreadCallHandler     handler,
                      void *                    params)
{
    flm_Thread *    thread;

    if (thread_pool->count == 0) {
        return (0);
    }

    if (to >= thread_pool->count) {
        return (-1);
    }

    thread = thread_pool->threads[to];
    if (flm_ThreadCall (thread, handler, params) == -1) {
        return (-1);
    }

    return (0);
}
