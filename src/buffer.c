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
#include "flm/core/private/error.h"
#include "flm/core/private/buffer.h"

flm_Buffer *
flm_BufferNew (char *                           content,
               size_t                           len,
               flm_BufferFreeContentHandler     fr_handler)
{
    flm_Buffer *        buffer;

    if ((buffer = flm__Alloc (sizeof (flm_Buffer))) == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        return (NULL);
    }
    flm__BufferInitRaw (buffer, content, len, fr_handler);
    return (buffer);
}

flm_Buffer *
flm_BufferView (flm_Buffer *    from,
                off_t           off,
                size_t          count)
{
    flm_Buffer *        buffer;

    if ((buffer = flm__Alloc (sizeof (flm_Buffer))) == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        return (NULL);
    }
    flm__BufferInitView (buffer, from, off, count);
    return (buffer);
}

flm_Buffer *
flm_BufferVPrintf (const char *         format,
                   va_list              ap)
{
    flm_Buffer *        buffer;

    char *              content;
    int                 len;
    size_t              alloc;

    va_list             ap_copy; /* needed on 64-bits platforms */

    va_copy (ap_copy, ap);
    len = vsnprintf (NULL, 0, format, ap_copy);
    va_end (ap_copy);

    alloc = len + 1;

    content = flm__Alloc (alloc * sizeof (char));
    if (content == NULL) {
        flm__Error = FLM_ERR_NOMEM;
        goto error;
    }

    va_copy (ap_copy, ap);
    vsnprintf (content, alloc, format, ap_copy);
    va_end (ap_copy);

    if ((buffer = flm_BufferNew (content, len, flm__Free)) == NULL) {
        goto free_content;
    }

    return (buffer);

free_content:
    flm__Free (content);
error:
    return (NULL);
}

flm_Buffer *
flm_BufferPrintf (const char *          format,
		  ...)
{
    flm_Buffer *        buffer;
    va_list             ap;

    va_start (ap, format);
    buffer = flm_BufferVPrintf (format, ap);
    va_end (ap);
    return (buffer);
}

size_t
flm_BufferLength (flm_Buffer *          buffer)
{
    switch (buffer->type) {
    case FLM__BUFFER_TYPE_RAW:
        return (buffer->content.raw.len);
    case FLM__BUFFER_TYPE_VIEW:
        return (buffer->content.view.count);
    default:
        break;
    }
    return (0);
}

char *
flm_BufferContent (flm_Buffer *         buffer)
{
    switch (buffer->type) {
    case FLM__BUFFER_TYPE_RAW:
        return (buffer->content.raw.content);
    case FLM__BUFFER_TYPE_VIEW:
        return (&(flm_BufferContent (
                      buffer->content.view.from)[buffer->content.view.off]));
    default:
        break;
    }
    return (NULL);
}

flm_Buffer *
flm_BufferRetain (flm_Buffer *          buffer)
{
    return (flm__Retain(&buffer->obj));
}

void
flm_BufferRelease (flm_Buffer *         buffer)
{
    flm__Release (&buffer->obj);
    return ;
}

void
flm__BufferInitRaw (flm_Buffer *                   buffer,
                    char *                         content,
                    size_t                         len,
                    flm_BufferFreeContentHandler   fr_handler)
{
    flm__ObjInit (&buffer->obj);

    buffer->obj.type = FLM__TYPE_BUFFER;

    buffer->obj.perf.destruct =                                 \
        (flm__ObjPerfDestruct_f) flm__BufferPerfDestruct;

    buffer->type = FLM__BUFFER_TYPE_RAW;
    buffer->content.raw.len = len;
    buffer->content.raw.content = content;
    buffer->content.raw.fr.handler = fr_handler;
    return ;
}

void
flm__BufferInitView (flm_Buffer *                  buffer,
                     flm_Buffer *                  from,
                     off_t                         off,
                     size_t                        count)
{
    flm__ObjInit (&buffer->obj);

    buffer->obj.type = FLM__TYPE_BUFFER;

    buffer->obj.perf.destruct =                                 \
        (flm__ObjPerfDestruct_f) flm__BufferPerfDestruct;

    buffer->type = FLM__BUFFER_TYPE_VIEW;
    buffer->content.view.from = flm_BufferRetain (from);
    buffer->content.view.off = off;
    buffer->content.view.count = count;
    return ;
}

void
flm__BufferPerfDestruct (flm_Buffer *   buffer)
{
    switch (buffer->type) {
    case FLM__BUFFER_TYPE_RAW:
        if (buffer->content.raw.fr.handler) {
            buffer->content.raw.fr.handler (buffer->content.raw.content);
        }
        break;
    case FLM__BUFFER_TYPE_VIEW:
        flm_BufferRelease (buffer->content.view.from);
        break;
    default:
        break;
    }
    return ;
}
