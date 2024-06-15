#ifndef MY_SECMALLOC_H
#define MY_SECMALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEMORY_SIZE 10000
#define CANARY_VALUE 0xDEADBEEF

typedef struct block {
    size_t size;
    size_t canary;
    struct block* next;
} block_t;

extern block_t* free_list; // Déclaration de free_list

extern char memory[MEMORY_SIZE];

void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t nmemb, size_t size);
void* my_realloc(void* ptr, size_t size);

#endif // MY_SECMALLOC_PRIVATE_H

