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

#if defined(linux)
#define _GNU_SOURCE
#include <sys/socket.h>
#endif

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <sys/sendfile.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/ssl.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/buffer.h"
#include "flm/core/private/error.h"
#include "flm/core/private/file.h"
#include "flm/core/private/io.h"
#include "flm/core/private/monitor.h"
#include "flm/core/private/obj.h"
#include "flm/core/private/stream.h"

const char * flm__StreamErrors[] =
{
    "Object is not shared",
    "Not implemented"
};

flm_Stream *
flm_StreamNew (flm_Monitor *		monitor,
	       int			fd,
	       void *		state)
{
    flm_Stream * stream;

    if ((stream = flm__Alloc (sizeof (flm_Stream))) == NULL) {
        return (NULL);
    }
    if (flm__StreamInit (stream, monitor, fd, state) == -1) {
        flm__Free (stream);
        return (NULL);
    }
    return (stream);
}

int
flm_StreamPrintf (flm_Stream *	stream,
		  const char *	format,
		  ...)
{
    flm_Buffer * buffer;

    char * content;
    int len;
    size_t alloc;

    va_list ap;

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

    if (flm_StreamPushBuffer (stream, buffer, 0, 0) == -1) {
        goto release_buffer;
    }
    flm_BufferRelease (buffer);

    return (0);

  release_buffer:
    flm_BufferRelease (buffer);
    return (-1);
  free_content:
    flm__Free (content);
  error:
    return (-1);
}

int
flm_StreamPushBuffer (flm_Stream *	stream,
		      flm_Buffer *	buffer,
		      off_t		off,
		      size_t		count)
{
    struct flm__StreamInput * input;

    if (off < 0) {
        off = 0;
    }
    if (off > (off_t) buffer->len) {
        off = buffer->len;
    }
    if (count == 0 || count > buffer->len) {
        count = buffer->len - off;
    }

    if ((input = flm__Alloc (sizeof (struct flm__StreamInput))) == NULL) {
        goto error;
    }

    ((flm_IO *)(stream))->wr.want = true;
    if (((flm_IO *)(stream))->monitor &&                            \
        flm__MonitorIOReset (((flm_IO *)(stream))->monitor,
                             (flm_IO *) stream) == -1) {
        goto free_input;
    }

    input->class.buffer = flm_BufferRetain (buffer);
    input->type = FLM__STREAM_TYPE_BUFFER;
    input->off = off;
    input->count = count;
    TAILQ_INSERT_TAIL (&(stream->inputs), input, entries);

    return (0);

  free_input:
    flm__Free (input);
  error:
    return (-1);
}

int
flm_StreamPushFile (flm_Stream *	stream,
		    flm_File *		file,
		    off_t		off,
		    size_t		count)
{
    struct flm__StreamInput * input;
    struct stat stat;

    if (count == 0) {
        if (fstat (((flm_IO *)(file))->sys.fd, &stat) == -1) {
            return (-1);
        }
        if (off > stat.st_size) {
            off = stat.st_size;
        }
        count = stat.st_size - off;
    }
    
    if ((input = flm__Alloc (sizeof (struct flm__StreamInput))) == NULL) {
        goto error;
    }
    if (((flm_IO *)(stream))->monitor &&                           \
        flm__MonitorIOReset (((flm_IO *)(stream))->monitor,        \
                             (flm_IO *) stream) == -1) {
        goto free_input;
    }	
    
    input->class.file = flm_FileRetain (file);
    input->type = FLM__STREAM_TYPE_BUFFER;
    input->off = off;
    input->count = count;
    TAILQ_INSERT_TAIL (&(stream->inputs), input, entries);
    
    ((flm_IO *)(stream))->wr.want = true;
    return (0);
    
  free_input:
    flm__Free (input);
  error:
    return (-1);
}

void
flm_StreamShutdown (flm_Stream *        stream)
{
    flm_IOShutdown ((flm_IO *) stream);
}

void
flm_StreamClose (flm_Stream *           stream)
{
    flm_IOClose ((flm_IO *) stream);
}

void
flm_StreamOnRead (flm_Stream *		stream,
		  flm_StreamReadHandler	handler)
{
    stream->rd.handler = handler;
    return ;
}

void
flm_StreamOnWrite (flm_Stream *			stream,
		   flm_StreamWriteHandler	handler)
{
    stream->wr.handler = handler;
    return ;
}

void
flm_StreamOnClose (flm_Stream *                 stream,
                   flm_StreamCloseHandler       handler)
{
    flm_IOOnClose ((flm_IO *) stream, (flm_IOCloseHandler) handler);
}

void
flm_StreamOnError (flm_Stream *                 stream,
                   flm_StreamErrorHandler       handler)
{
    flm_IOOnError ((flm_IO *) stream, (flm_IOErrorHandler) handler);
}

int
flm_StreamStartTLSServer (flm_Stream *               stream,
                          SSL_CTX *                  context)
{
    /**
     * TODO: don't forget to flush the existing buffers, since an
     * attacker could easily inject some clear-text data right after the
     * TLS negociation.
     */
    if (flm__StreamInitTLS (stream, context) == -1) {
        goto error;
    }

    if (SSL_accept (stream->tls.obj) < 0) {
        goto shutdown_tls;
    }

    return (0);
    
  shutdown_tls:
    flm__StreamShutdownTLS (stream);
  error:
    return (-1);
}

flm_Stream *
flm_StreamRetain (flm_Stream * stream)
{
    return (flm__Retain ((flm_Obj *) stream));
}

void
flm_StreamRelease (flm_Stream * stream)
{
    flm__Release ((flm_Obj *) stream);
    return ;
}

int
flm__StreamInit (flm_Stream *	stream,
		 flm_Monitor *	monitor,
		 int		fd,
		 void *		state)
{
    if (flm__IOInit ((flm_IO *) stream, monitor, fd, state) == -1) {
        return (-1);
    }
    ((flm_Obj *)(stream))->type = FLM__TYPE_STREAM;

    ((flm_Obj *)(stream))->perf.destruct =                      \
        (flm__ObjPerfDestruct_f) flm__StreamPerfDestruct;

    ((flm_IO *)(stream))->perf.read	=                       \
        (flm__IOSysRead_f) flm__StreamPerfRead;

    ((flm_IO *)(stream))->perf.write	=                       \
        (flm__IOSysWrite_f) flm__StreamPerfWrite;

    TAILQ_INIT (&stream->inputs);
    
    flm_StreamOnRead (stream, NULL);
    flm_StreamOnWrite (stream, NULL);
    
    stream->tls.obj = NULL;
    
    return (0);
}

int
flm__StreamInitTLS (flm_Stream *                stream,
                    SSL_CTX *                   context)
{
    stream->tls.obj = SSL_new (context);
    if (stream->tls.obj == NULL) {
        goto error;
    }
    if (!SSL_set_fd (stream->tls.obj, ((flm_IO *)(stream))->sys.fd)) {
        goto free_obj;
    }
    return (0);

  free_obj:
    SSL_free (stream->tls.obj);
    stream->tls.obj = NULL;
  error:
    return (-1);
}

void
flm__StreamShutdownTLS (flm_Stream *    stream)
{
    if (stream->tls.obj) {
        SSL_free (stream->tls.obj);
        stream->tls.obj = NULL;
    }
    return ;
}

void
flm__StreamPerfDestruct (flm_Stream * stream)
{
    struct flm__StreamInput * input;
    struct flm__StreamInput temp;

    /* remove remaining stuff to write */
    TAILQ_FOREACH (input, &stream->inputs, entries) {
        temp.entries = input->entries;
        flm__Release (input->class.obj);
        TAILQ_REMOVE (&stream->inputs, input, entries);
        flm__Free (input);
        input = &temp;
    }
    flm__StreamShutdownTLS (stream);
    return ;
}

void
flm__StreamPerfRead (flm_Stream *	stream,
		     flm_Monitor *	monitor,
		     uint8_t		count)
{
    struct iovec iovec[count];
    unsigned int iov_count;
    unsigned int drain_count;

    ssize_t nb_read;
    size_t drain;

    flm_Buffer * buffer;
    size_t iov_read;

    (void) monitor;

    for (iov_count = 0; iov_count < count; iov_count++) {

        iovec[iov_count].iov_base = flm__Alloc(FLM_STREAM__RBUFFER_SIZE);

        if (iovec[iov_count].iov_base == NULL) {
            /* no more memory */
            count = iov_count;
            break ;
        }
        iovec[iov_count].iov_len = FLM_STREAM__RBUFFER_SIZE;
    }
    if (iov_count == 0) {
        return ;
    }

    nb_read = readv (((flm_IO *)(stream))->sys.fd, iovec, iov_count);

    drain_count = 0;
    if (nb_read == 0) {
        /* close */
        ((flm_IO *)(stream))->rd.can = 0;
        flm_IOShutdown ((flm_IO *) stream);
        goto out;
    }
    else if (nb_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /**
             * the kernel buffer was empty
             */
            ((flm_IO *)(stream))->rd.can = 0;
        }
        else if (errno == EINTR) {
            /**
             * interrupted by a signal, just retry
             */
            ((flm_IO *)(stream))->rd.can = 1;
        }
        else {
            /* fatal error */
            flm__Error = FLM_ERR_ERRNO;
            flm_IOClose ((flm_IO *) stream);
            /**
             * Call the error handler
             */
            if (((flm_IO *)(stream))->er.handler) {
                ((flm_IO *)(stream))->er.handler ((flm_IO *) stream,
                                                  ((flm_IO *)(stream))->state,
                                                  flm_Error());
            }
        }
        goto out;
    }
    /* read event */
    ((flm_IO *)(stream))->rd.can = 1;
    for (drain = nb_read; drain; drain_count++) {

        iov_read = drain < iovec[drain_count].iov_len ?		\
            drain : iovec[drain_count].iov_len;

        if ((buffer = flm_BufferNew (iovec[drain_count].iov_base, \
                                     iov_read,			  \
                                     flm__Free)) == NULL) {
            /**
             * Call the error handler
             */
            if (((flm_IO *)(stream))->er.handler) {
                ((flm_IO *)(stream))->er.handler ((flm_IO *) stream,
                                                  ((flm_IO *)(stream))->state,
                                                  flm_Error());
            }
            goto out;
        }

        /**
         * Call the read handler with the new buffer
         */
        if (stream->rd.handler) {
            stream->rd.handler (stream,
                                ((flm_IO *)(stream))->state,
                                buffer);
        }

        if (drain < iovec[drain_count].iov_len) {
            ((flm_IO *)(stream))->rd.can = 0;
            drain = 0;
        }
        else {
            drain -= iovec[drain_count].iov_len;
        }
    }

  out:
    /* free remaining iov_bases */
    for (iov_count = drain_count; iov_count < count; iov_count++) {
        flm__Free (iovec[iov_count].iov_base);
    }

    return ;
}

void
flm__StreamPerfWrite (flm_Stream *	stream,
		      flm_Monitor *	monitor,
		      uint8_t		count)
{
    struct flm__StreamInput * input;
    struct flm__StreamInput temp;
    ssize_t nb_write;
    ssize_t drain;

    (void) monitor;
    (void) count;

    nb_write = flm__StreamWrite (stream);

    if (nb_write < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* kernel buffer was full */
            ((flm_IO *)(stream))->wr.can = 0;
            return ;
        }
        else if (errno == EINTR) {
            /* interrupted by a signal, just retry */
            ((flm_IO *)(stream))->wr.can = 1;
            return ;
        }
        else {
            /* fatal error */
            flm__Error = FLM_ERR_ERRNO;
            flm_IOClose ((flm_IO *) stream);

            /**     
             * Call the error handler   
             */
            if (((flm_IO *)(stream))->er.handler) {
                ((flm_IO *)(stream))->er.handler ((flm_IO *) stream,
                                                  ((flm_IO *)(stream))->state,
                                                  flm_Error());
            }
            return ;
        }
    }
                
    /**
     * Call the write handler with the number of bytes written
     */
    if (stream->wr.handler) {
        stream->wr.handler (stream,
                            ((flm_IO *)(stream))->state,
                            nb_write);
    }

    drain = nb_write;
    ((flm_IO *)(stream))->wr.can = 1;
    TAILQ_FOREACH (input, &(stream->inputs), entries) {
        if (drain == 0) {
            break ;
        }
        if (drain < input->tried) {
            input->off += drain;
            input->count -= drain;
            ((flm_IO *)(stream))->wr.can = 0;
            break ;
        }

        drain -= input->tried;

        temp.entries = input->entries;
        flm__Release (input->class.obj);
        TAILQ_REMOVE (&(stream->inputs), input, entries);
        flm__Free (input);
        input = &temp;
    }

    if (TAILQ_FIRST (&stream->inputs) == NULL) {
        ((flm_IO *)(stream))->wr.want = false;
    }
    return ;
}

ssize_t
flm__StreamWrite (flm_Stream * stream)
{
    struct flm__StreamInput * input;
    ssize_t nb_write;

    if ((input = TAILQ_FIRST (&stream->inputs)) == NULL) {
        return (0);
    }

    nb_write = 0;
    switch (input->type) {
    case FLM__STREAM_TYPE_BUFFER:
        nb_write = flm__StreamSysWritev (stream);
        break ;

    case FLM__STREAM_TYPE_FILE:
#if defined (linux)
        nb_write = flm__StreamSysSendFile (stream);
#else
        nb_write = flm__StreamSysReadWriteTo (stream);
#endif
        break ;
    }
    return (nb_write);
}

ssize_t
flm__StreamSysWritev (flm_Stream * stream)
{
    struct flm__StreamInput * input;

    struct iovec iovec[FLM_STREAM__IOVEC_SIZE];
    size_t iov_count;

    iov_count = 0;
    TAILQ_FOREACH (input, &stream->inputs, entries) {
        if (iov_count == FLM_STREAM__IOVEC_SIZE) {
            break ;
        }
        if (input->type != FLM__STREAM_TYPE_BUFFER) {
            break ;
        }

        iovec[iov_count].iov_base = &input->class.buffer->content[input->off];
        iovec[iov_count].iov_len = input->tried = input->count;
        iov_count++;
    }
    return (writev (((flm_IO *)(stream))->sys.fd, iovec, iov_count));
}

ssize_t
flm__StreamSysSendFile (flm_Stream * stream)
{
    struct flm__StreamInput * input;

    off_t off;

    if ((input = TAILQ_FIRST (&stream->inputs)) == NULL) {
        return (0);
    }

    off = input->off;
    input->tried = input->count;
    return (sendfile (((flm_IO *)(stream))->sys.fd,              \
                      ((flm_IO *)(input->class.file))->sys.fd,   \
                      &off,					 \
                      input->tried));
}
