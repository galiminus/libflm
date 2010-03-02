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

#include <string.h>
#include <pthread.h>

#include "flm/core/extern/slabs.h"

#include "flm/core/private/error.h"
#include "flm/core/private/alloc.h"

static pthread_mutex_t flm__InitSlabMutex = PTHREAD_MUTEX_INITIALIZER;
static int flm__IsSlabInit = 0;
static int flm__IsSlabDisabled = 0;

#define FLM_ALLOC_SIZE(size) ((size) + sizeof (struct flm__AllocSlabHeader))

static inline void *
flm__AllocSetSlabHeader (unsigned char * mem,
		       unsigned int clsid,
		       unsigned int size)
{
	struct flm__AllocSlabHeader * header;

	header = (struct flm__AllocSlabHeader *)mem;
	header->clsid = clsid;
	header->size = size;
	return (&(mem[sizeof (struct flm__AllocSlabHeader)]));
}

static inline struct flm__AllocSlabHeader *
flm__AllocGetSlabHeader (unsigned char * mem)
{
	struct flm__AllocSlabHeader * header;

	header = (struct flm__AllocSlabHeader *)	\
		&(mem[-sizeof (struct flm__AllocSlabHeader)]);

	return (header);
}

static int
init_slab ()
{
	const char * slab_disable_env;

	if (pthread_mutex_lock(&flm__InitSlabMutex) == -1) {
		flm__Error = FLM_ERR_ERRNO;
		return (-1);
	}
	if (!flm__IsSlabInit) {
		slab_disable_env = getenv (FLM_DISABLE_SLAB_ALLOCATOR_ENV);
		if (slab_disable_env && strcasecmp (slab_disable_env, "no")) {
			flm__IsSlabDisabled = 1;
		}
		else {
			flm__SlabsExternInit (FLM__SLABS_BASE, FLM__SLABS_FACTOR);
		}
		flm__IsSlabInit = 1;
	}
	if (pthread_mutex_unlock(&flm__InitSlabMutex) == -1) {
		flm__Error = FLM_ERR_ERRNO;
		return (-1);
	}
	return (0);
}

void *
flm_SlabAlloc (size_t size)
{
	void * mem;
	unsigned int clsid;
	size_t n_size;

	if (!flm__IsSlabInit && init_slab () == -1) {
		return (NULL);
	}

	if (flm__IsSlabDisabled) {
		return (flm__MemAlloc (size));
	}

	n_size = FLM_ALLOC_SIZE (size);
	if ((clsid = flm__SlabsExternClsid (n_size)) == 0) {
		return (NULL);
	}
	if ((mem = flm__SlabsExternAlloc (n_size, clsid)) == NULL) {
		return (NULL);
	}
	mem = flm__AllocSetSlabHeader (mem, clsid, n_size);
	return (mem);
}

void
flm_SlabFree (void * mem)
{
	struct flm__AllocSlabHeader * header;

	if (flm__IsSlabDisabled) {
		flm__Free (mem);
		return ;
	}
	if (mem == NULL) {
		return ;
	}
	header = flm__AllocGetSlabHeader (mem);
	flm__SlabsExternFree (header, header->size, header->clsid);
	return ;
}

void *
flm__MemAlloc (size_t size)
{
	void * mem;

	mem = malloc (size);
	if (mem == NULL) {
		flm__Error = FLM_ERR_ERRNO;
		return (NULL);
	}
	return (mem);
}

void *
flm__ReAlloc (void * mem,
	      size_t size)
{
	mem = realloc (mem, size);
	if (mem == NULL) {
		flm__Error = FLM_ERR_ERRNO;
		return (NULL);
 	}
	return (mem);
}

void
flm__Free (void * mem)
{
	free (mem);
	return ;
}
