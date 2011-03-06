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

#ifndef _FLM_CORE_PUBLIC_FILE_H_
# define _FLM_CORE_PUBLIC_FILE_H_

typedef struct flm_File flm_File;

#include <sys/types.h>

#include "flm/core/public/obj.h"

flm_File *
flm_FileNew (int fd);

/**
 * \brief Open a file.
 *
 * See fopen(3) for further explanations about the parameters.
 *
 * \param path A path to a file in the file system.
 * \param flags File flags as described in fopen(3).
 * \return A pointer to a new \c flm_file obj.
 * \retval NULL in case of error, usually when
 *   there is no more memory available or you cannot access to
 *   the file.
 */
flm_File *
flm_FileOpen (const char * root, const char * path, const char * mode);

flm_File *
flm_FileRetain (flm_File * file);

void
flm_FileRelease (flm_File * file);

#endif /* _FLM_CORE_PUBLIC_FILE_H_ */
