/* From memcached : http://www.danga.com/memcached/ */

/* slabs memory allocation */
#ifndef _FLM_CORE_EXTERN_SLABS_H_
# define _FLM_CORE_EXTERN_SLABS_H_

/**
 * Init the subsystem. 1st argument is the limit on no. of bytes to allocate,
 * 0 if no limit. 2nd argument is the growth factor; each slab will use a
 * chunk size equal to the previous slab's chunk size times this factor.
 * 3rd argument specifies if the slab allocator should allocate all memory
 * up front (if true), or allocate memory in chunks as it is needed (if false)
 */
void
flm__SlabsExternInit (size_t size,
		      const double factor);

/**
 * Given obj size, return id to use when allocating/freeing memory for
 * obj 0 means error: can't store such a large obj
 */
unsigned int
flm__SlabsExternClsid(const size_t size);

/* Allocate obj of given length. 0 on error */ /*@null@*/
void *
flm__SlabsExternAlloc(const size_t size, unsigned int id);

/**
 * Free previously allocated obj
 */
void
flm__SlabsExternFree(void *ptr, size_t size, unsigned int id);

#endif /* !_FLM_CORE_EXTERN_SLABS_H_ */
