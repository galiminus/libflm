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

#ifndef _FLM_CORE_PUBLIC_STREAM_H_
# define _FLM_CORE_PUBLIC_STREAM_H_

#include <stdint.h>

#include <openssl/ssl.h>

typedef struct flm_Stream flm_Stream;

#include "flm/core/public/buffer.h"
#include "flm/core/public/container.h"
#include "flm/core/public/file.h"
#include "flm/core/public/monitor.h"
#include "flm/core/public/obj.h"

typedef void (*flm_StreamReadHandler)			\
(flm_Stream * stream, void * state, flm_Buffer * buffer);

typedef void (*flm_StreamWriteHandler)			\
(flm_Stream * stream, void * state, size_t size);

typedef void (*flm_StreamCloseHandler)                  \
(flm_Stream * stream, void * state);

typedef void (*flm_StreamErrorHandler)                  \
(flm_Stream * stream, void * state, int error);

flm_Stream *
flm_StreamNew (flm_Monitor *	monitor,	\
	       int		fd,		\
	       void *		state);

int
flm_StreamPrintf (flm_Stream *	stream,
		  const char *	format,
		  ...);

int
flm_StreamPushBuffer (flm_Stream *	stream,
		      flm_Buffer *	buffer,
		      off_t		off,
		      size_t		count);

int
flm_StreamPushFile (flm_Stream *	stream,
		    flm_File *		file,
		    off_t		off,
		    size_t		count);

void
flm_StreamShutdown (flm_Stream *        stream);

void
flm_StreamClose (flm_Stream *           stream);

void
flm_StreamOnRead (flm_Stream *		stream,
		  flm_StreamReadHandler	handler);

void
flm_StreamOnWrite (flm_Stream *			stream,
		   flm_StreamWriteHandler	handler);

void
flm_StreamOnClose (flm_Stream *                 stream,
                   flm_StreamCloseHandler       handler);

void
flm_StreamOnError (flm_Stream *                 stream,
                   flm_StreamErrorHandler       handler);

int
flm_StreamStartTLSServer (flm_Stream *		stream,
                          SSL_CTX *             context);

int
flm_StreamStartTLSClient (flm_Stream *		stream,
                          SSL_CTX *             context);

flm_Stream *
flm_StreamRetain (flm_Stream * stream);

void
flm_StreamRelease (flm_Stream * stream);

#endif /* _FLM_CORE_PUBLIC_STREAM_H_ */
