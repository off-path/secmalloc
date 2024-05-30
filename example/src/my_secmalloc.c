#define _GNU_SOURCE
#include "my_secmalloc.private.h"
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#define MEMORY_SIZE 10000

typedef struct block {
    size_t size;
    struct block* next;
} block_t;

block_t* free_list = NULL;
char memory[MEMORY_SIZE];

void* my_malloc(size_t size)
{
    //Check minimum size
    if (size == 0 || size > MEMORY_SIZE) {
        return NULL;
    }

    //Search free memory block with sufisent size
    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current->size < size) {
        prev = current;
        current = current->next;
    }

    //If no block with sufisent size found return NULL
    if (current == NULL) {
        return NULL;
    }
    
    //If the block is too big divided by to 2
    if (current->size > size + sizeof(block_t)) {
        block_t* new_block = (block_t*)((char*)current + size);
        new_block->size = current->size - size - sizeof(block_t);
        new_block->next = current->next;
        current->size = size;
        current->next = new_block;
    }
    
    //Update free block
    if (prev == NULL) {
        free_list = current->next;
    }
    else {
        prev->next = current->next;
    }

    //Return pointer to the start of allocated block
    return (void*)current;
}
void my_free(void* ptr)
{
    //Check if the ptr is valid
    if (ptr == NULL){
        return;
    }

    // Check if the ptr is on allocated memory with my_malloc()
    if (((char*)ptr < memory) || ((char*)ptr > memory + MEMORY_SIZE)) {
        return;
    }

    //Retreive if the block is associated to a pointer
    block_t* block = (block_t*)ptr - 1;

    //Check if memory block is valided
    if (block->size == 0) {
        return;
    }
    // Add the block to the list of free block
    block->next = free_list;
    free_list = block;

    //Renitialize allocated memory with memset() to avoid other memory leak
    memset(ptr, 0, block->size);
}

void* my_calloc(size_t nmemb, size_t size)
{
    (void)nmemb;
    (void)size;

    //Valid parametes check
    if (nmemb == 0 || size == 0){
        return NULL;
    }

    //Calculate the total memory size to allocate
    size_t total_size = nmemb * size;

    //Check total memory size is not too big
    if (total_size / nmemb != size){
        return NULL;
    } 

    //Alloc memory with my_alloc()
    void* ptr = my_malloc(total_size);
    if(ptr == NULL){
        return NULL;
    }

    // Initilise memory allocated to 0 with memset()
    memset(ptr, 0, total_size);
    
    //Return
    return ptr;
}

void* my_realloc(void* ptr, size_t size)
{
    (void)ptr;
    (void)size;
    return NULL;
}

#ifdef DYNAMIC
void* malloc(size_t size)
{
    return my_malloc(size);
}
void    free(void* ptr)
{
    my_free(ptr);
}
void* calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

void* realloc(void* ptr, size_t size)
{
    return my_realloc(ptr, size);

}

#endif
