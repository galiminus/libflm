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

#include <sys/types.h>

#include <string.h>

#include "flm/core/private/alloc.h"

#include "flm/core/private/map.h"

static int flm__MapExpand (flm__Map * map, size_t index);

flm__Map *
flm__MapNew (size_t alloc)
{
	flm__Map * map;

	map = flm_SlabAlloc (sizeof (flm__Map));
	if (map == NULL) {
		return (NULL);
	}
	if (flm__MapInit (map, alloc) == -1) {
		flm_SlabFree (map);
		return (NULL);
	}
	return (map);
}

size_t
flm__MapSize (flm__Map * map)
{
	return (map->alloc);
}

int
flm__MapSet (flm__Map * map,
	     size_t index,
	     void * filter)
{
	if (index >= map->alloc && (flm__MapExpand (map, index) == -1)) {
		return (-1);
	}
	map->filters[index] = filter;
	return (0);
}

void *
flm__MapGet (flm__Map * map,
	    size_t index)
{
	if (index >= map->alloc) {
		return (NULL);
	}
	return (map->filters[index]);
}

void *
flm__MapRemove (flm__Map * map,
	       size_t index)
{
	void * filter;

	filter = flm__MapGet (map, index);
	if (filter == NULL) {
		return (NULL);
	}
	map->filters[index] = NULL;
	return (filter);
}

int
flm__MapInit (flm__Map * map,
	       size_t alloc)
{
	if (flm__ObjInit (FLM_OBJ (map)) == -1) {
		goto error;
	}
	FLM_OBJ (map)->type = FLM__TYPE_MAP;

	FLM_OBJ (map)->perf.destruct =				\
		(flm__ObjPerfDestruct_f) flm__MapPerfDestruct;

	map->alloc = 0;
	map->filters = NULL;

	if (alloc == 0) {
		alloc = FLM__MAP_BASE_SIZE;
	}
	if (flm__MapExpand (map, alloc) == -1) {
		goto error;
	}

	return (0);

error:
	return (-1);
}

void
flm__MapPerfDestruct (flm__Map * map)
{
	flm__Free (map->filters);
	return ;
}

int
flm__MapExpand (flm__Map * map,
		size_t index)
{
	size_t nalloc;
	void ** filters;

	if (map->alloc) {
		nalloc = (((index) / map->alloc) + 1) * map->alloc;
	}
	else {
		nalloc = index;
	}

        filters = flm__ReAlloc (map->filters, nalloc * sizeof (void *));
	if (filters == NULL) {
		return (-1);
	}
	memset (&filters[map->alloc], 0, (nalloc - map->alloc) * sizeof (void *));

	map->filters = filters;
	map->alloc = nalloc;

	return (0);
}
