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

#ifndef _FLM_CORE_PRIVATE_ERROR_H_
# define _FLM_CORE_PRIVATE_ERROR_H_

#include "flm/core/public/error.h"

#define flm__Error (*(flm__ErrorLocation()))

#define	FLM__STRERR_UNKNOWN	"Unknown error"
#define FLM__STRERR_NOMEM       "No more memory available"
#define FLM__STRERR_NOSYS       "Mandatory system call is unavailable"
#define FLM__STRERR_BUG         "Bug discovered"

#define FLM__ERROR_MAX_DOMAINS	128

int *
flm__ErrorLocation ();

void
flm__ErrorAdd (int domain, const char ** errors);

const char *
flm__ErrorDesc (int domain, int error);

#endif /* !_FLM_CORE_PRIVATE_ERROR_H_ */
