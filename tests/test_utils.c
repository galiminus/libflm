#include "flm/flm.h"

size_t alloc_sum;
size_t alloc_count;
size_t alloc_current;

size_t
getAllocSum ()
{
    return (alloc_sum);
}


void *
testAllocHandler (size_t size)
{
    size_t total_size;
    size_t * ptr;

    if (alloc_current == alloc_count) {
        alloc_current++;
        return (NULL);
    }

    total_size = size + sizeof (size);

    alloc_current++;
    alloc_sum += size;

    ptr = malloc (total_size);
    if (ptr == NULL) {
        return (NULL);
    }

    ptr[0] = size;

    return ((void *)(ptr + 1));
}

void
testFreeHandler (void * ptr)
{
    size_t * size_ptr;
    size_t size;

    size_ptr = ptr;
    size = size_ptr[0];

    alloc_sum -= size;

    free (size_ptr + 1);
    return ;
}

void *
testReallocHandler (void * ptr, size_t size)
{
    void * new_ptr;

    new_ptr = testAllocHandler (size);
    memcpy (new_ptr, ptr, size);
    testFreeHandler (ptr);
    return (new_ptr);
}

void
setTestAlloc (uint32_t count)
{
    alloc_count = count;
    alloc_current = 1;
    alloc_sum = 0;

    flm__SetAlloc (testAllocHandler);
    flm__SetRealloc (testReallocHandler);
    flm__SetFree (testAllocHandler);

    return ;
}
