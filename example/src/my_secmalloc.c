#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "my_secmalloc.private.h"

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define MPT MAP_PRIVATE|MAP_ANONYMOUS
#define PRT PROT_WRITE|PROT_READ

// The union Premap_s is already defined in the included header file.
// Remove the redundant definition here.

// Remove this redundant code
// typedef union Premap_s {
//     uintptr_t align;
//     size_t allocsize;
// } Premap;

static size_t rndup(size_t size) {
    int pgs = getpagesize() - 1;
    size += sizeof(Premap);
    size += pgs;
    size &= ~pgs;
    return size;
}

void *malloc(size_t size) {
    if (!size) return NULL;
    size = rndup(size);
    Premap *map = mmap(NULL, size, PRT, MPT, -1, 0);
    if (map == MAP_FAILED) return NULL;
    map->allocsize = size;
    return &map[1];
}

void free(void *ptr) {
    if (!ptr) return;
    Premap *map = (ptr - sizeof(Premap));
    size_t size = map->allocsize;
    munmap(map, size);
}

void *calloc(size_t nmemb, size_t size) {
    size *= nmemb;
    void *ptr = malloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    size_t oldsz, rsize;
    Premap *map, *nmap;

    if (!size) {
        free(ptr);
        return NULL;
    }
    if (!ptr) return malloc(size);
    rsize = rndup(size);
    map = (ptr - sizeof(Premap));
    oldsz = map->allocsize;
    if (oldsz < rsize) { /* Agrandi */
        nmap = mmap(NULL, rsize, PRT, MPT, -1, 0);
        if (nmap == MAP_FAILED) return NULL;
        memcpy(nmap, map, oldsz);
        nmap->allocsize = rsize;
        free(ptr);
        return &nmap[1];
    } else if (oldsz > rsize) { /* Réduit */
        nmap = mmap(NULL, rsize, PRT, MPT, -1, 0);
        if (nmap == MAP_FAILED) return ptr;
        memcpy(nmap, map, rsize);
        nmap->allocsize = rsize;
        free(ptr);
        return &nmap[1];
    }
    return ptr;
}
