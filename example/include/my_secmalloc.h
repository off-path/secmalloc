#ifndef MY_SECMALLOC_PRIVATE_H
#define MY_SECMALLOC_PRIVATE_H

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define MEMORY_SIZE 10000
#define CANARY_VALUE 0xDEADBEEF

typedef struct block {
    size_t size;
    size_t canary;
    struct block* next;
} block_t;

extern block_t* free_list;
extern char memory[MEMORY_SIZE];

void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t nmemb, size_t size);
void* my_realloc(void* ptr, size_t size);

#endif // MY_SECMALLOC_PRIVATE_H

