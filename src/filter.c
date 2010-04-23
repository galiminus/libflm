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

#include <sys/stat.h>

#include <stdio.h>
#include <stdarg.h>

#include "flm/core/private/alloc.h"
#include "flm/core/private/buffer.h"
#include "flm/core/private/filter.h"
#include "flm/core/private/file.h"
#include "flm/core/private/io.h"

int
flm_FilterAppendBuffer (flm_Filter * filter,
			flm_Buffer * buffer,
			off_t off,
			size_t count)
{
	struct flm__FilterInput * input;

	if (off < 0) {
		off = 0;
	}
	if (off > (off_t) buffer->len) {
		off = buffer->len;
	}
	if (count == 0 || count > buffer->len) {
		count = buffer->len - off;
	}

	if ((input = flm__Alloc (sizeof (struct flm__FilterInput))) == NULL) {
		return (-1);
	}
	input->class.buffer = flm__Retain (FLM_OBJ (buffer));
	input->type = FLM__FILTER_TYPE_BUFFER;
	input->off = off;
	input->count = count;
	TAILQ_INSERT_TAIL (&(filter->inputs), input, entries);
	return (0);
}

int
flm_FilterAppendFile (flm_Filter * filter,
		      flm_File * file,
		      off_t off,
		      size_t count)
{
	struct flm__FilterInput * input;
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

	if ((input = flm__Alloc (sizeof (struct flm__FilterInput))) == NULL) {
		return (-1);
	}
	input->class.file = flm__Retain (FLM_OBJ (file));
	input->type = FLM__FILTER_TYPE_BUFFER;
	input->off = off;
	input->count = count;
	TAILQ_INSERT_TAIL (&(filter->inputs), input, entries);
	return (0);
}

int
flm__FilterInit (flm_Filter * filter)
{
	if (flm__ObjInit (FLM_OBJ (filter)) == -1) {
		return (-1);
	}

	FLM_OBJ (filter)->perf.destruct =				\
		(flm__ObjPerfDestruct_f) flm__FilterPerfDestruct;

	TAILQ_INIT (&filter->inputs);
	return (0);
}

void
flm__FilterPerfDestruct (flm_Filter * filter)
{
	struct flm__FilterInput * input;
	struct flm__FilterInput temp;

	/* remove remaining stuff to write */
	TAILQ_FOREACH (input, &filter->inputs, entries) {
		temp.entries = input->entries;
		flm__Release (input->class.obj);
		TAILQ_REMOVE (&filter->inputs, input, entries);
		flm__Free (input);
		input = &temp;
	}
	return ;
}
