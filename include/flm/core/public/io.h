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

#ifndef _FLM_CORE_PUBLIC_IO_H_
# define _FLM_CORE_PUBLIC_IO_H_

#include <stdint.h>

typedef struct flm_IO flm_IO;

#include "flm/core/public/monitor.h"
#include "flm/core/public/obj.h"

#define FLM_IO(_obj) FLM_CAST(_obj, flm_IO)

typedef void (*flm_IOReadHandler) (flm_IO * io, void * data, size_t count);
typedef void (*flm_IOWriteHandler) (flm_IO * io, void * data, size_t count);

/**
 * \brief Close the \c flm_io obj as soon as possible.
 *
 * All the data to write will be written before closing the underlying
 * file descriptor. You will never receive read notifications anymore.
 *
 * \param io A pointer to a \c flm_io obj.
 * \return Nothing, this function cannot fail.
 */
void
flm_IOShutdown (flm_IO * io);

void
flm_IOClose (flm_IO * io);

#endif /* _FLM_CORE_PUBLIC_IO_H_ */