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
 * \file alloc.h
 * Like many other libraries, libflm provides some wrappers around the
 * most commons memory-management functions (malloc(3), realloc(3) and free(3)),
 * the main purpose is to implement a tracing system above these function
 * keep track of allocations and desallocations.
 * libflm alse provides a slab allocator based on the memcached's allocator,
 * using the slab allocator is faster but consumes much more memory.
 * Internaly, libflm use the slab allocator for each obj creation.
 *
 * You can disable the slab allocator by setting the \c LIBFLM_DISABLE_SLAB_ALLOCATOR
 * environment variable to a value different than 'no'. This can be useful
 * for low-memory systems.
 */

#ifndef _FLM_CORE_PRIVATE_ALLOC_H_
# define _FLM_CORE_PRIVATE_ALLOC_H_

#include <stdlib.h>

#include "flm/core/public/alloc.h"

#define FLM__SLABS_BASE		64
#define FLM__SLABS_FACTOR	1.25

struct flm__AllocSlabHeader {
	unsigned int	clsid;
	size_t		size;
};

/**
 * \brief Simple wrapper around malloc(3).
 *
 * \param size Size to allocate (in byte), this size has to be greater than 1
 *   and lower than INT_MAX.
 * \return A pointer to the newly allocated memory area.
 * \retval NULL in case of error.
 */
void *
flm__MemAlloc (unsigned int size);

/**
 * \brief Simple wrapper around realloc(3)
 *
 * \param mem A pointer to a memory area allocated with flm__malloc(3)
 *   or flm_realloc(3).
 * \param size Size to allocate (in byte), this size has to be greater than 1
 *   and lower than INT_MAX.
 * \return A pointer, possibly the same than the pointer provided,
 *   to the memory area.
 * \retval NULL in case of error.
 */
void *
flm__ReAlloc (void * mem, unsigned int size);

/**
 * \brief Simple wrapper around flm__free(3).
 *
 * \param mem A pointer to a memory area allocated with flm__malloc()
 * or flm_realloc().
 * \return flm__free() returns nothing.
 */
void
flm__Free (void * mem);

#endif /* !_FLM_CORE_PRIVATE_ALLOC_H_ */
