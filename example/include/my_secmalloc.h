#ifndef _SECMALLOC_H
#define _SECMALLOC_H

#include <stddef.h>
#define MEMORY_SIZE 10000
void    *malloc(size_t size);
void    free(void *ptr);
void    *calloc(size_t nmemb, size_t size);
void    *realloc(void *ptr, size_t size);

#endif
