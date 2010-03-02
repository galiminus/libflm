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

#ifndef _FLM_CORE_PRIVATE_MAP_H_
# define _FLM_CORE_PRIVATE_MAP_H_

typedef struct flm__Map flm__Map;

#include "flm/core/private/obj.h"

#define FLM__TYPE_MAP	0x000C0000

struct flm__Map
{
	/* inheritance */
	struct flm_Obj		obj;

	/* members */
	void **			filters;
	size_t			alloc;
};

#define FLM__MAP_BASE_SIZE	64

typedef void (*flm__MapFree_f)(void *);

/**
 * \brief Travers a map.
 *
 * This macro acts like a for{} loop:
 * \code
 * flm_buf * my_filter;
 * size_t index;
 *
 * FLM_MAP_FOREACH(my_map, my_filter, index) {
 *  printf ("[%d]: %s\n", index, flm_buf_src (my_filter));
 * }
 * \endcode
 *
 * \param map a pointer to a \c flm_map obj.
 * \param filter A pointer to the filterent type, this pointer will set
 *   to a new filterent from the map for each iteration.
 * \param index An unsigned int used to keep track of the index.
 */
#define FLM_MAP_FOREACH(map, filter, index)				\
	for ((index) = 0, (filter = flm__MapGet((map), (index)));		\
	     index < flm__MapSize (map);				\
	     ++(index), (filter = flm__MapGet((map), (index))))

/**
 * \brief Remove each filterent of a map one by one.
 *
 * This macro acts like a for{} loop:
 * \code
 * flm_buf * my_filter;
 * size_t index;
 *
 * FLM_MAP_REMOVE_EACH(my_map, my_filter) {
 *  printf ("%s\n", flm_buf_src (my_filter));
 * }
 * \endcode
 *
 * \param map a pointer to a \c flm_map obj.
 * \param filter A pointer to the filterent type, this pointer will set
 *   a new filterent from the map for each iteration.
 * \param index An unsigned int used to keep track of the index.
 */
#define FLM_MAP_REMOVE_EACH(map, filter, index)			\
	for ((index) = 0;					\
	     (filter = flm__MapSize ((map), (index))) != NULL;	\
	     (index++))

flm__Map *
flm__MapNew (size_t alloc);

size_t
flm__MapSize (flm__Map * map);

/**
 * \brief Add an obj at the end of the map.
 *
 * The obj will be referenced by the map, so if you don't need
 * it after this call, don't forget to release it with flm_release() to
 * avoid memory leaks.
 * \code
 * flm_buf * buf;
 * flm__Map map;
 *
 * buf = flm_buf_new ();
 * flm_buf_printf (buf, "Good bye world !");
 * map = flm_map_new ();
 * flm_map_add (map, FLM_OBJ (buf));
 * flm_release (buf);
 * \endcode
 * \param map a pointer to a \c flm_map obj.
 * \param filter A pointer to an filterent.
 * \return An error code.
 * \retval 0 in case of success.
 * \retval -1 in case of error.
 */
int
flm__MapSet (flm__Map * map, size_t index, void * filter);

/**
 * \brief Get an obj from the map.
 *
 * Note that the obj will still be referenced by the map, the
 * reference counter will be incremented and the obj stays
 * in the map.
 *
 * You might be annoyed by the \c index argument, since flm_map_add()
 * doesn't take any index. In fact, flm_map_get is mostly use to
 * travers a map (see flm_map_foreach()) so an index is needed to
 * keep an eye on the current filterent.
 *
 * \param map a pointer to a \c flm_map obj
 * \param index The index of the obj
 * \return A pointer to the filterent.
 * \retval NULL in case of error.
 */
void *
flm__MapGet (flm__Map * map, size_t index);

/**
 * \brief Remove the first filterent of the map.
 *
 * The obj will be deleted from the map.
 *
 * \param map a pointer to a \c flm_map obj
 * \return An pointer to an filterent.
 * \retval -1 in case of error.
 */
void *
flm__MapRemove (flm__Map * map, size_t index);

int
flm__MapInit (flm__Map * map, size_t alloc);

void
flm__MapPerfDestruct (flm__Map * map);

#endif /* !_FLM_CORE_PRIVATE_MAP_H_ */
