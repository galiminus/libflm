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

#include "flm/core/public/container.h"

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

	return (0);

release_buffer:
	flm_Release (FLM_OBJ (buffer));
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

	FLM_IO (stream)->wr.want = true;
	if (FLM_IO (stream)->monitor &&					\
	    flm__MonitorIOReset (FLM_IO (stream)->monitor, FLM_IO (stream)) == -1) {
		goto free_input;
	}

	input->class.buffer = flm_Retain (FLM_OBJ (buffer));
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
		if (fstat (FLM_IO (file)->sys.fd, &stat) == -1) {
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
	if (FLM_IO (stream)->monitor &&					\
	    flm__MonitorIOReset (FLM_IO (stream)->monitor, FLM_IO (stream)) == -1) {
		goto free_input;
	}	

	input->class.file = flm_Retain (FLM_OBJ (file));
	input->type = FLM__STREAM_TYPE_BUFFER;
	input->off = off;
	input->count = count;
	TAILQ_INSERT_TAIL (&(stream->inputs), input, entries);

	FLM_IO (stream)->wr.want = true;
	return (0);

free_input:
	flm__Free (input);
error:
	return (-1);
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

int
flm__StreamInit (flm_Stream *			stream,
		 flm_Monitor *			monitor,
		 int				fd,
		 void *		state)
{
	if (flm__IOInit (FLM_IO (stream), monitor, fd, state) == -1) {
		return (-1);
	}
	FLM_OBJ (stream)->type = FLM__TYPE_STREAM;

	FLM_OBJ (stream)->perf.destruct =			\
		(flm__ObjPerfDestruct_f) flm__StreamPerfDestruct;

	FLM_IO (stream)->perf.read	=			\
		(flm__IOSysRead_f) flm__StreamPerfRead;

	FLM_IO (stream)->perf.write	=			\
		(flm__IOSysWrite_f) flm__StreamPerfWrite;

	TAILQ_INIT (&stream->inputs);

	flm_StreamOnRead (stream, NULL);
	flm_StreamOnWrite (stream, NULL);

	return (0);
}

void
flm__StreamPerfDestruct (flm_Stream * stream)
{
	struct flm__StreamInput * input;
	struct flm__StreamInput temp;

	/* remove remaining stuff to write */
	TAILQ_FOREACH (input, &stream->inputs, entries) {
		temp.entries = input->entries;
		flm_Release (input->class.obj);
		TAILQ_REMOVE (&stream->inputs, input, entries);
		flm__Free (input);
		input = &temp;
	}
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

	nb_read = readv (FLM_IO (stream)->sys.fd, iovec, iov_count);

	drain_count = 0;
	if (nb_read == 0) {
		/* close */
		FLM_IO (stream)->rd.can = 0;
		flm_IOShutdown (FLM_IO (stream));
		goto out;
	}
	else if (nb_read < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			/* kernel buffer was empty */
			FLM_IO (stream)->rd.can = 0;
		}
		else if (errno == EINTR) {
			/* interrupted by a signal, just retry */
			FLM_IO (stream)->rd.can = 1;
		}
		else {
			/* fatal error */
			flm__Error = FLM_ERR_ERRNO;
			flm_IOClose (FLM_IO (stream));
			FLM_IO_EVENT_WITH (FLM_IO (stream), er, flm_Error());
		}
		goto out;
	}
	/* read event */
	FLM_IO (stream)->rd.can = 1;
	for (drain = nb_read; drain; drain_count++) {

		iov_read = drain < iovec[drain_count].iov_len ?		\
			drain : iovec[drain_count].iov_len;

		if ((buffer = flm_BufferNew (iovec[drain_count].iov_base, \
					     iov_read,			  \
					     flm__Free)) == NULL) {
			FLM_IO_EVENT_WITH (FLM_IO (stream), er, flm_Error());
			goto out;
		}

		FLM_IO_EVENT_WITH (stream, rd, buffer);

		if (drain < iovec[drain_count].iov_len) {
			FLM_IO (stream)->rd.can = 0;
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

	nb_write = flm__StreamWrite (stream);

	if (nb_write < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			/* kernel buffer was full */
			FLM_IO (stream)->wr.can = 0;
			return ;
		}
		else if (errno == EINTR) {
			/* interrupted by a signal, just retry */
			FLM_IO (stream)->wr.can = 1;
			return ;
		}
		else {
			/* fatal error */
			flm__Error = FLM_ERR_ERRNO;
			flm_IOClose (FLM_IO (stream));
			FLM_IO_EVENT_WITH (FLM_IO (stream), er, flm_Error());
			return ;
		}
	}

	/* write event */

	drain = nb_write;
	FLM_IO (stream)->wr.can = 1;
	TAILQ_FOREACH (input, &(stream->inputs), entries) {
		if (drain == 0) {
			break ;
		}
		if (drain < input->tried) {
			input->off += drain;
			input->count -= drain;
			FLM_IO (stream)->wr.can = 0;
			break ;
		}

		drain -= input->tried;

		temp.entries = input->entries;
		flm_Release (input->class.obj);
		TAILQ_REMOVE (&(stream->inputs), input, entries);
		flm__Free (input);
		input = &temp;
	}

	if (TAILQ_FIRST (&stream->inputs) == NULL) {
		FLM_IO (stream)->wr.want = false;
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

	struct msghdr msg;
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
	return (writev (FLM_IO (stream)->sys.fd, iovec, iov_count));
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
	return (sendfile (FLM_IO (stream)->sys.fd,		 \
			  FLM_IO (input->class.file)->sys.fd,	 \
			  &off,					 \
			  input->tried));
}
