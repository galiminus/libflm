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

#ifndef _FLM_CORE_PUBLIC_ERROR_H_
# define _FLM_CORE_PUBLIC_ERROR_H_

enum flm_error
{
	/**
	 * System error, you should take a look at errno.
	 */
	FLM_ERR_ERRNO =         0x0000,

        /**
         * No more memory available.
         */
        FLM_ERR_NOMEM,

        /**
         * The needed syscalls are not avaible on your system
         */
        FLM_ERR_NOSYS,

        /**
         * A bug was discovered, it would be very nice of you to send a report :)
         */
        FLM_ERR_BUG
};

/**
 * \brief Return last error code.
 *
 * All error codes are defined in the documentation.
 * The error code depends on the last operation performed, note that
 * the function is fully threadsafe and use per-thread variable.
 *
 * \return The last error code, this function itself cannot fail.
 */
int
flm_Error (void);

#endif /* !_FLM_CORE_PUBLIC_ERROR_H_ */
