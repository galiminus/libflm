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

#ifndef _FLM_CORE_PRIVATE_FILE_H_
# define _FLM_CORE_PRIVATE_FILE_H_

#include <sys/types.h>
#include <sys/stat.h>

#include "flm/core/public/file.h"

#include "flm/core/private/io.h"

#define FLM__TYPE_FILE	0x00030000

#define FLM_FILE(_obj) FLM_CAST(_obj, flm_File)

struct flm_File
{
	/* inheritance */
	struct flm_IO		io;
};

int
flm__FileInit (flm_File * file, int fd);

int
flm__FileInitOpen (flm_File * file,					\
		   const char * root, const char * path, const char * mode);

#endif /* !_FLM_CORE_PRIVATE_FILE_H_ */
