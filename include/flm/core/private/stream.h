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

#ifndef _FLM_CORE_PRIVATE_STREAM_H_
# define _FLM_CORE_PRIVATE_STREAM_H_

#include <sys/queue.h>
#include <sys/uio.h>

#include <stdint.h>
#include <stdbool.h>

#include "flm/core/public/stream.h"
#include "flm/core/public/monitor.h"

#include "flm/core/private/io.h"

#define FLM__TYPE_STREAM	0x000F0000

struct flm__StreamInput
{
	union {
		flm_Obj *		obj;
		flm_File *		file;
		flm_Buffer *		buffer;
	} class;
	enum {
					FLM__STREAM_TYPE_FILE,
					FLM__STREAM_TYPE_BUFFER
	} type;

	off_t				off;
	size_t				count;
	int				tried;
	TAILQ_ENTRY (flm__StreamInput)	entries;
};

struct flm_Stream
{
	/* inheritance */
	struct flm_IO				io;

	struct {
		flm_StreamFeedHandler		handler;
	} fe;
	struct {
		flm_StreamReadHandler		handler;
	} rd;
	struct {
		flm_StreamWriteHandler		handler;
	} wr;

	TAILQ_HEAD (flin, flm__StreamInput)	inputs;
};

#define FLM_STREAM__RBUFFER_SIZE		2048
#define FLM_STREAM__IOVEC_SIZE			8

int
flm__StreamInit (flm_Stream *			stream,
		 flm_Monitor *			monitor,
		 int				fd,
		 void *				state);

void
flm__StreamPerfDestruct (flm_Stream * stream);

void
flm__StreamPerfRead (flm_Stream * stream,
		     flm_Monitor * monitor,
		     uint8_t count);

void
flm__StreamPerfWrite (flm_Stream * stream,
		      flm_Monitor * monitor,
		      uint8_t count);

ssize_t
flm__StreamWrite (flm_Stream * stream);

ssize_t
flm__StreamSysWritev (flm_Stream * stream);

ssize_t
flm__StreamSysSendFile (flm_Stream * stream);

#endif /* !_FLM_CORE_PRIVATE_STREAM_H_ */
