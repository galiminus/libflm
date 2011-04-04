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

/**
 * \brief A special container for chunks of memory.
 */

/**
 * \file buffer.h
 * \c The flm_Buffer class is a simple container for memory chunks, it
 * holds the content of this chunk, its length, and a handler to free
 * it. This handler will be called when the reference counter of the
 * buffer reaches zero. For example, if you allocated a buffer using
 * malloc() the free handler will be the free() function.
 */

#ifndef _FLM_CORE_PUBLIC_BUFFER_H_
# define _FLM_CORE_PUBLIC_BUFFER_H_

#ifndef _FLM__SKIP

#include <unistd.h>
#include <stdarg.h>

typedef struct flm_Buffer flm_Buffer;

#include "flm/core/public/obj.h"

#endif /* _FLM__SKIP */

/**
 * A pointer to simple 'free' handler, the libc's free() function can be
 * used directly if the content of the buffer was created with malloc().
 */
typedef void (*flm_BufferFreeContentHandler)(void * ptr);

/**
 * \brief Create a new buffer from an existing chunk of memory.
 *
 * \param content A pointer to any array of characters.
 * \param length The lenght (in bytes) of this array.
 * \param fr_handler A pointer to a desallocation function.
 *
 * \return A pointer to a new flm_Buffer object.
 *
 * \par Example with static string:
 * \code
 * flm_Buffer * buffer;
 *
 * buffer = flm_BufferNew ("test", 5, NULL);
 * \endcode
 *
 * \par Example with dynamic string:
 * \code
 * flm_Buffer * buffer;
 * char *       string;
 *
 * string = strdup ("test");
 * buffer = flm_BufferNew (string, 5, free);
 * \endcode
 *
 */
flm_Buffer *
flm_BufferNew (char *				content,
	       size_t				length,
	       flm_BufferFreeContentHandler	fr_handler);

flm_Buffer *
flm_BufferView (flm_Buffer *                    buffer,
                off_t                           off,
                size_t                          count);

/**
 * \brief Create a new flm_Buffer object and fill it with a formatted
 * string.
 *
 * This function is the equivalent of printf() for
 * buffers. Internally, it uses the vsnprintf() function so all the
 * formatting rules are the same.
 *
 * \remark You should never use an user input as format string, see the BUGS
 * section of the printf() manual page.
 *
 * \param format A printf()-style format string.
 *
 * \return A pointer to a new flm_Buffer object.
 *
 * \par Example:
 * \code
 *
 *  flm_Buffer * buffer;
 *
 *  buffer = flm_BufferPrintf ("TEST %d %s ", 42, "coucou");
 *  assert (strcmp (flm_BufferContent (buffer), "TEST 42 coucou ") == 0);
 *
 * \endcode
 */
flm_Buffer *
flm_BufferPrintf (const char *                  format, ...);

/**
 * \brief Returns the length (in bytes) of the buffer.
 *
 * \remark Note that buffers created with flm_BufferPrintf() do not count the
 * ending \0 in the total length, even if this character is present at
 * the end of the buffer content.
 *
 * \param buffer A pointer to a flm_Buffer object.
 *
 * \return The length of the buffer in bytes.
 */
size_t
flm_BufferLength (flm_Buffer * buffer);

/**
 * \brief Returns the content of the buffer.
 *
 * \remark Note that buffers created with flm_BufferPrintf() will have an
 * ending \0, even is this character is not included in the total
 * length.
 *
 * \param buffer A pointer to a flm_Buffer object.
 *
 * \return The content of the buffer.
 */
char *
flm_BufferContent (flm_Buffer * buffer);

/**
 * \brief Increment the reference counter.
 *
 * \param buffer A pointer to a flm_Buffer object.
 *
 * \return The same pointer, this function cannot fail.
 */
flm_Buffer *
flm_BufferRetain (flm_Buffer *  buffer);

/**
 * \brief Decrement the reference counter.
 *
 * When the counter reaches zero the 'free' handler will be called to
 * free the memory consumed by the buffer content, then the buffer
 * object itself will be freed.
 *
 * \param buffer A pointer to flm_Buffer object..
 */
void
flm_BufferRelease (flm_Buffer * buffer);

/**
 * \example "Buffer unit tests"
 * \include buffer_test.c
 */

#endif /* !_FLM_CORE_PUBLIC_BUFFER_H_ */
