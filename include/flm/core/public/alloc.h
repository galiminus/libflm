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
 * libflm provides a slab allocator based on the memcached's allocator,
 * using the slab allocator is faster but consumes much more memory.
 * Internaly, libflm use the slab allocator for each object creation.
 *
 * You can disable the slab allocator by setting the \c FLM_DISABLE_SLAB_ALLOCATOR
 * environment variable to a value different than 'no'. This can be useful
 * for low-memory systems or for debugging purpose (memory leaks wont show up
 * on valgrind with the slab allocator activated).
 */

#ifndef _FLM_CORE_PUBLIC_ALLOC_H_
# define _FLM_CORE_PUBLIC_ALLOC_H_

#define FLM_DISABLE_SLAB_ALLOCATOR_ENV "FLM_DISABLE_SLAB_ALLOCATOR"

/**
 * \brief Allocate a slab of size \c size.
 *
 * The slab allocator is really useful when you want to allocate a
 * lot of same-sized structures. Allocating a slab is faster and doesn't
 * suffer from fragmentation but uses a lot more memory.
 *
 * \param size Size to allocate (in byte), this size has to be greater than 1
 *   and lower than INT_MAX.
 * \return A pointer to the newly allocated memory area.
 * \retval NULL in case of error.
 */
void *
flm_SlabAlloc (size_t size);

/**
 * \brief Free a previously allocated slab.
 *
 * \param mem A pointer to a slab allocated with flm_SlabAlloc().
 * \return flm_SlabFree() returns nothing.
 */
void
flm_SlabFree (void * mem);

#endif /* !_FLM_CORE_PUBLIC_ALLOC_H_ */
