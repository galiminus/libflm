#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <netdb.h>

#include <assert.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>

#include <check.h>

#include "flm/flm.h"

#include "flm/core/private/alloc.h"

size_t alloc_sum;
size_t alloc_count;
size_t alloc_current;

#define DEBUG 0

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

    if (DEBUG) {
        printf ("Alloc: %d\n", size);
    }

    alloc_current++;
    alloc_sum += size;

    ptr = malloc (total_size);
    if (ptr == NULL) {
        return (NULL);
    }

    ptr[0] = size;
    return ((void *)(&ptr[1]));
}

void
testFreeHandler (void * ptr)
{
    size_t * size_ptr;
    size_t size;

    size_ptr = ptr;
    size = size_ptr[-1];

    if (DEBUG) {
        printf ("Free: %d\n", size);
    }

    alloc_sum -= size;

    free (&size_ptr[-1]);
    return ;
}

void
setTestAlloc (uint32_t count)
{
    alloc_count = count;
    alloc_current = 1;
    alloc_sum = 0;

    flm__SetAlloc (testAllocHandler);
    flm__SetFree (testFreeHandler);

    if (DEBUG) {
        printf ("---\n");
    }

    return ;
}

int
getFDCount ()
{
    int fd;

    fd = open ("/dev/null", O_RDONLY);
    close(fd);
    return (fd);
}

int
testConnect (char * host, int port)
{
    int                 sock;
    struct sockaddr_in  addr;
    struct hostent *    addrinfo;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return (-1);
    }

    addrinfo = gethostbyname (host);
    if (addrinfo == NULL) {
        return (-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);

    memcpy (&(addr.sin_addr), addrinfo->h_addr, addrinfo->h_length);

    if (connect (sock,
                 (struct sockaddr *) &addr,
                 sizeof (struct sockaddr_in)) == -1) {
        printf("%s\n", strerror(errno));
        return (-1);
    }
    close (sock);
    return (0);
}
