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
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "flm/core/public/file.h"

#include "flm/core/private/alloc.h"
#include "flm/core/private/buffer.h"

flm_Buffer *
flm_BufferNew (char *                           content,
               size_t                           len,
               flm_BufferFreeContentHandler     fr_handler)
{
    flm_Buffer *        buffer;

    if ((buffer = flm__Alloc (sizeof (flm_Buffer))) == NULL) {
        return (NULL);
    }
    flm__BufferInit (buffer, content, len, fr_handler);
    return (buffer);
}

flm_Buffer *
flm_BufferPrintf (const char *          format,
		  ...)
{
    flm_Buffer *        buffer;

    char *              content;
    int                 len;
    size_t              alloc;

    va_list             ap;

    va_start (ap, format);
    len = vsnprintf (NULL, 0, format, ap);
    va_end (ap);
    
    if (len < 0) {
        goto error;
    }
    
    alloc = len + 1;
    
    content = flm__Alloc (alloc * sizeof (char));
    if (content == NULL) {
        goto error;
    }
    
    va_start (ap, format);
    len = vsnprintf (content, alloc, format, ap);
    va_end (ap);
    
    if (len < 0) {
        goto free_content;
    }
    
    if ((buffer = flm_BufferNew (content, len, flm__Free)) == NULL) {
        goto free_content;
    }
    
    return (buffer);

free_content:
    flm__Free (content);
error:
    return (NULL);
}

size_t
flm_BufferLength (flm_Buffer *          buffer)
{
    return (buffer->len);
}

char *
flm_BufferContent (flm_Buffer *         buffer)
{
    return (buffer->content);
}

flm_Buffer *
flm_BufferRetain (flm_Buffer *          buffer)
{
    return (flm__Retain ((flm_Obj *) buffer));
}

void
flm_BufferRelease (flm_Buffer *         buffer)
{
    flm__Release ((flm_Obj *) buffer);
    return ;
}

void
flm__BufferInit (flm_Buffer *                   buffer,
                 char *                         content,
                 size_t                         len,
                 flm_BufferFreeContentHandler   fr_handler)
{
    flm__ObjInit (FLM_OBJ (buffer));

    FLM_OBJ (buffer)->type = FLM__TYPE_BUFFER;

    FLM_OBJ (buffer)->perf.destruct =                               \
        (flm__ObjPerfDestruct_f) flm__BufferPerfDestruct;

    buffer->len = len;
    buffer->content = content;

    buffer->fr.handler = fr_handler;
    return ;
}

void
flm__BufferPerfDestruct (flm_Buffer *   buffer)
{
    if (buffer->fr.handler) {
        buffer->fr.handler (buffer->content);
    }
    return ;
}
