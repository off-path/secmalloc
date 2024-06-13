#ifndef __MY_ALLOC_PRIVATE_H
#define __MY_ALLOC_PRIVATE_H

#include "my_secmalloc.h"
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

typedef union Premap_s {
    uintptr_t align;
    size_t allocsize;
} Premap;

#endif