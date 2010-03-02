/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/* From memcached : http://www.danga.com/memcached/ */

/*
 * Slabs memory allocation, based on powers-of-N. Slabs are up to 1MB in size
 * and are divided into chunks. The chunk sizes start off at the size of the
 * "item" structure plus space for a small key and value. They increase by
 * a multiplier factor from there, up to half the maximum slab size. The last
 * slab size is always 1MB, since that's the maximum item size allowed by the
 * memcached protocol.
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Slab sizing definitions. */
#define POWER_SMALLEST 1
#define POWER_LARGEST  200
#define POWER_BLOCK 1048576
#define CHUNK_ALIGN_BYTES 8
#define MAX_NUMBER_OF_SLAB_CLASSES (POWER_LARGEST + 1)

/* powers-of-N allocation structures */

typedef struct {
    unsigned int size;      /* sizes of items */
    unsigned int perslab;   /* how many items per slab */

    void **slots;           /* list of item ptrs */
    unsigned int sl_total;  /* size of previous array */
    unsigned int sl_curr;   /* first free slot */

    void *end_page_ptr;         /* pointer to next free item at end of page, or 0 */
    unsigned int end_page_free; /* number of items remaining at end of last alloced page */

    unsigned int slabs;     /* how many slabs were allocated for this class */

    void **slab_list;       /* array of slab pointers */
    unsigned int list_size; /* size of prev array */

    unsigned int killing;  /* index+1 of dying slab, or zero if none */
    size_t requested; /* The number of requested bytes */
} slabclass_t;

static slabclass_t slabclass[MAX_NUMBER_OF_SLAB_CLASSES];
static size_t mem_limit = 0;
static size_t mem_malloced = 0;
static int power_largest;

static void *mem_base = NULL;
static void *mem_current = NULL;
static size_t mem_avail = 0;

/**
 * Access to the slab allocator is protected by this lock
 */
static pthread_mutex_t slabs_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Forward Declarations
 */
static int
do_slabsNewslab(const unsigned int id);

static void *
memory_allocate(size_t size);

/**
 * Figures out which slab class (chunk size) is required to store an item of
 * a given size.
 *
 * Given obj size, return id to use when allocating/freeing memory for
 * obj 0 means error: can't store such a large obj
 */
unsigned int
flm__SlabsExternClsid (const size_t size)
{
    int res;

    res = POWER_SMALLEST;
    if (size == 0) {
        return (0);
    }
    while (size > slabclass[res].size) {
        if (res++ == power_largest) { /* won't fit in the biggest slab */
            return (0);
        }
    }
    return (res);
}

/**
 * Determines the chunk sizes and initializes the slab class descriptors
 * accordingly.
 */
void
flm__SlabsExternInit (size_t size,
                      const double factor)
{
    int i;

    /* Factor of 2.0 means use the default memcached behavior */
    if (factor == 2.0 && size < 128) {
        size = 128;
    }

    memset (slabclass, 0, sizeof (slabclass));

    i = POWER_SMALLEST - 1;
    while (++i < POWER_LARGEST && size <= POWER_BLOCK / 2) {
        /* Make sure items are always n-byte aligned */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        slabclass[i].size = size;
        slabclass[i].perslab = POWER_BLOCK / slabclass[i].size;
        size *= factor;
    }

    power_largest = i;
    slabclass[power_largest].size = POWER_BLOCK;
    slabclass[power_largest].perslab = 1;
    return ;
}

static int
grow_slab_list (const unsigned int id)
{
    slabclass_t * p;
    size_t new_size;
    void * new_list;

    p = &slabclass[id];
    if (p->slabs == p->list_size) {
        new_size =  (p->list_size != 0) ? p->list_size * 2 : 16;
        new_list = realloc (p->slab_list, new_size * sizeof (void *));
        if (new_list == 0) {
            return (0);
        }
        p->list_size = new_size;
        p->slab_list = new_list;
    }
    return (1);
}

static int do_slabsNewslab(const unsigned int id) {
    slabclass_t *p = &slabclass[id];
    int len = p->size * p->perslab;
    char *ptr;

    if ((mem_limit && mem_malloced + len > mem_limit && p->slabs > 0) ||
        (grow_slab_list(id) == 0) ||
        ((ptr = memory_allocate((size_t)len)) == 0)) {

        return 0;
    }

    memset(ptr, 0, (size_t)len);
    p->end_page_ptr = ptr;
    p->end_page_free = p->perslab;

    p->slab_list[p->slabs++] = ptr;
    mem_malloced += len;

    return 1;
}

/*@null@*/
static void *
do_slabs_alloc (const size_t size,
                unsigned int id)
{
    slabclass_t * p;
    void * ret;

    ret = NULL;
    if (id < POWER_SMALLEST || id > (unsigned int)(power_largest)) {
        return (NULL);
    }

    p = &slabclass[id];

    /* fail unless we have space at the end of a recently allocated page,
       we have something on our freelist, or we could allocate a new page */
    if (! (p->end_page_ptr != 0 || p->sl_curr != 0 ||   \
           do_slabsNewslab(id) != 0)) {
        /* We don't have more memory available */
        ret = NULL;
    }
    else if (p->sl_curr != 0) {
        /* return off our freelist */
        ret = p->slots[--p->sl_curr];
    }
    else {
        /* if we recently allocated a whole page, return from that */
        ret = p->end_page_ptr;
        if (--p->end_page_free != 0) {
            p->end_page_ptr = ((caddr_t)p->end_page_ptr) + p->size;
        }
        else {
            p->end_page_ptr = 0;
        }
    }

    if (ret) {
        p->requested += size;
    }

    return ret;
}

static void
do_slabs_free(void * ptr,
              const size_t size,
              unsigned int id)
{
    slabclass_t * p;
    int new_size;
    void ** new_slots;

    if (id < POWER_SMALLEST || id > (unsigned int)(power_largest)) {
        return ;
    }

    p = &slabclass[id];

    if (p->sl_curr == p->sl_total) { /* need more space on the free list */
        /* 16 is arbitrary */
        new_size = (p->sl_total != 0) ? p->sl_total * 2 : 16;
        new_slots = realloc(p->slots, new_size * sizeof(void *));
        if (new_slots == 0) {
            return ;
        }
        p->slots = new_slots;
        p->sl_total = new_size;
    }
    p->slots[p->sl_curr++] = ptr;
    p->requested -= size;
    return ;
}

static void *
memory_allocate (size_t size)
{
    void * ret;

    if (mem_base == NULL) {
        /* We are not using a preallocated large memory chunk */
        ret = malloc(size);
    }
    else {
        ret = mem_current;

        if (size > mem_avail) {
            return (NULL);
        }

        /* mem_current pointer _must_ be aligned!!! */
        if (size % CHUNK_ALIGN_BYTES) {
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);
        }

        mem_current = ((char*)mem_current) + size;
        if (size < mem_avail) {
            mem_avail -= size;
        }
        else {
            mem_avail = 0;
        }
    }

    return ret;
}

void *
flm__SlabsExternAlloc (size_t size,
                       unsigned int id)
{
    void * ret;

    if (pthread_mutex_lock(&slabs_lock) == -1) {
        return (NULL);
    }
    ret = do_slabs_alloc(size, id);
    if (pthread_mutex_unlock(&slabs_lock) == -1) {
        return (NULL);
    }
    return ret;
}

void
flm__SlabsExternFree (void *ptr,
                      size_t size,
                      unsigned int id)
{
    if (pthread_mutex_lock(&slabs_lock) == -1) {
        return ;
    }
    do_slabs_free(ptr, size, id);
    if (pthread_mutex_unlock(&slabs_lock) == -1) {
        return ;
    }
    return ;
}
