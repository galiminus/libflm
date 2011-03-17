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
#include <pthread.h>

#include "flm/core/private/error.h"
#include "flm/core/private/alloc.h"

void * (*allocHandler)(size_t);
void   (*freeHandler)(void *);

void
flm__SetAlloc (void * (*handler)(size_t))
{
    allocHandler = handler;
}

void
flm__SetFree (void (*handler)(void *))
{
    freeHandler = handler;
}

void *
flm__Alloc (size_t      size)
{
    void * mem;

    if (allocHandler) {
        mem = allocHandler (size);
    }
    else {
        mem = malloc (size);
    }
    if (mem == NULL) {
        flm__Error = FLM_ERR_ERRNO;
        return (NULL);
    }
    return (mem);
}

void
flm__Free (void *       mem)
{
    if (freeHandler) {
        freeHandler (mem);
    }
    else {
        free (mem);
    }
    return ;
}
