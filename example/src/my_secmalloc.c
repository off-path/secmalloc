#define _GNU_SOURCE
#include "my_secmalloc.private.h"
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

void *my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Aligner la taille demandée sur la taille de page du système
    long page_size = sysconf(_SC_PAGESIZE);
    size_t total_size = (size + page_size - 1) & ~(page_size - 1); // arround to page zize >

    void* ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return ptr;
}

void my_free(void *ptr) {
    if (ptr == NULL) return; // si le pointeur est null, raf

    // En supposant que la taille de la mémoire à libérer est stockée juste avant le pointeur
    size_t* size_ptr = (size_t*)ptr - 1;
    munmap((void*)size_ptr, *size_ptr);
}

void *my_calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void* ptr = my_malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size); // init mem to 0
    }
    return ptr;
}


void *my_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return my_malloc(size);
    }
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    // get actual size block
    size_t* old_size = (size_t*)ptr - 1;

    // Allouez un nouveau bloc de mémoire avec la nouvelle taille
    void *new_ptr = my_malloc(size);
    if (new_ptr) {
        // Copiez old datas vers le nouveau
        memcpy(new_ptr, ptr, size < *old_size ? size : *old_size);
        my_free(ptr);
    }
    return new_ptr;
}


#ifdef DYNAMIC
void    *malloc(size_t size)
{
    return my_malloc(size);
}
void    free(void *ptr)
{
    my_free(ptr);
}
void    *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

void    *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);

}

#endif
