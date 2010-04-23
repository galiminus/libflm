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

#include "flm/core/private/alloc.h"
#include "flm/core/private/buffer.h"
#include "flm/core/private/error.h"
#include "flm/core/private/file.h"
#include "flm/core/private/io.h"
#include "flm/core/private/stream.h"
#include "flm/core/private/obj.h"

const char * flm__StreamErrors[] =
{
	"Object is not shared",
	"Not implemented"
};

flm_Stream *
flm_StreamNew (flm_Monitor *		monitor,
	       flm_StreamReadHandler	rd_handler,
	       flm_StreamWriteHandler	wr_handler,
	       flm_StreamCloseHandler	cl_handler,
	       flm_StreamErrorHandler	er_handler,
	       flm_StreamTimeoutHandler	to_handler,
	       void *			data,
	       int			fd,
	       uint32_t			timeout)
{
	flm_Stream * stream;

	if ((stream = flm__Alloc (sizeof (flm_Stream))) == NULL) {
		return (NULL);
	}
	if (flm__StreamInit (stream,					\
			     monitor,					\
			     rd_handler,				\
			     wr_handler,				\
			     cl_handler,				\
			     er_handler,				\
			     to_handler,				\
			     data,					\
			     fd,					\
			     timeout) == -1) {
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
	flm__Release (FLM_OBJ (buffer));
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
	if (flm_FilterAppendBuffer (FLM_FILTER (stream), buffer, off, count) == -1) {
		return (-1);
	}
	FLM_IO (stream)->wr.want = true;
	return (0);
}

int
flm_StreamPushFile (flm_Stream *	stream,
		    flm_File *		file,
		    off_t		off,
		    size_t		count)
{
	if (flm_FilterAppendFile (FLM_FILTER (stream), file, off, count) == -1) {
		return (-1);
	}
	FLM_IO (stream)->wr.want = true;
	return (0);
}

int
flm__StreamInit (flm_Stream *			stream,
		 flm_Monitor *			monitor,
		 flm_StreamReadHandler		rd_handler,
		 flm_StreamWriteHandler		wr_handler,
		 flm_StreamCloseHandler		cl_handler,
		 flm_StreamErrorHandler		er_handler,
		 flm_StreamTimeoutHandler	to_handler,
		 void *				data,
		 int				fd,
		 uint32_t			timeout)
{
	if (flm__IOInit (FLM_IO (stream),			\
			 monitor,				\
			 (flm__IOCloseHandler) cl_handler,	\
			 (flm__IOErrorHandler) er_handler,	\
			 (flm__IOTimeoutHandler) to_handler,	\
			 data,					\
			 fd,					\
			 timeout) == -1) {
		return (-1);
	}
	FLM_OBJ (stream)->type = FLM__TYPE_STREAM;

	stream->rd.handler		=	rd_handler;

	stream->wr.handler		=	wr_handler;

	FLM_IO (stream)->perf.read	=			\
		(flm__IOSysRead_f) flm__StreamPerfRead;

	FLM_IO (stream)->perf.write	=			\
		(flm__IOSysWrite_f) flm__StreamPerfWrite;

	return (0);
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
		iovec[iov_count].iov_base =				\
			flm__Alloc (FLM_STREAM__RBUFFER_SIZE);

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
			FLM_IO_EVENT (FLM_IO (stream), er, monitor);
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
			FLM_IO_EVENT (FLM_IO (stream), er, monitor);
			goto out;
		}

		FLM_IO_EVENT_WITH (stream, rd, monitor, buffer);

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
	struct flm__FilterInput * filter;
	struct flm__FilterInput temp;
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
			FLM_IO_EVENT (FLM_IO (stream), er, monitor);
			return ;
		}
	}

	/* write event */

	drain = nb_write;
	FLM_IO (stream)->wr.can = 1;
	TAILQ_FOREACH (filter, &(FLM_FILTER (stream)->inputs), entries) {
		if (drain == 0) {
			break ;
		}
		if (drain < filter->tried) {
			filter->off += drain;
			filter->count -= drain;
			FLM_IO (stream)->wr.can = 0;
			break ;
		}

		drain -= filter->tried;

		temp.entries = filter->entries;
		flm__Release (filter->class.obj);
		TAILQ_REMOVE (&(FLM_FILTER (stream)->inputs), filter, entries);
		flm__Free (filter);
		filter = &temp;
	}

	if (TAILQ_FIRST (&FLM_FILTER (stream)->inputs) == NULL) {
		FLM_IO (stream)->wr.want = false;
	}
	return ;
}

ssize_t
flm__StreamWrite (flm_Stream * stream)
{
	struct flm__FilterInput * filter;
	ssize_t nb_write;

	if ((filter = TAILQ_FIRST (&FLM_FILTER (stream)->inputs)) == NULL) {
		return (0);
	}

	nb_write = 0;
	switch (filter->type) {
	case FLM__FILTER_TYPE_BUFFER:
		nb_write = flm__StreamSysWritev (stream);
		break ;

	case FLM__FILTER_TYPE_FILE:
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
	struct flm__FilterInput * filter;

	struct msghdr msg;
	struct iovec iovec[FLM_STREAM__IOVEC_SIZE];
	size_t iov_count;

	iov_count = 0;
	TAILQ_FOREACH (filter, &FLM_FILTER (stream)->inputs, entries) {
		if (iov_count == FLM_STREAM__IOVEC_SIZE) {
			break ;
		}
		if (filter->type != FLM__FILTER_TYPE_BUFFER) {
			break ;
		}

		iovec[iov_count].iov_base = &filter->class.buffer->content[filter->off];
		iovec[iov_count].iov_len = filter->tried = filter->count;
		iov_count++;
	}
	return (writev (FLM_IO (stream)->sys.fd, iovec, iov_count));
}

ssize_t
flm__StreamSysSendFile (flm_Stream * stream)
{
	struct flm__FilterInput * filter;

	off_t off;

	if ((filter = TAILQ_FIRST (&FLM_FILTER (stream)->inputs)) == NULL) {
		return (0);
	}

	off = filter->off;
	filter->tried = filter->count;
	return (sendfile (FLM_IO (stream)->sys.fd,		 \
			  FLM_IO (filter->class.file)->sys.fd,	 \
			  &off,					 \
			  filter->tried));
}
