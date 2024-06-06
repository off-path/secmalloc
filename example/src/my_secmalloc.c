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
    // Check minimum size
    if (size == 0 || size > MEMORY_SIZE - sizeof(block_t)) return NULL;

    // Align size to the nearest multiple of sizeof(block_t)
    size = (size + sizeof(block_t) - 1) & ~(sizeof(block_t) - 1);

    // Initialize free_list if it's not already initialized
    if (free_list == NULL) {
        free_list = (block_t*)memory;
        free_list->size = MEMORY_SIZE - sizeof(block_t);
        free_list->next = NULL;
    }

    // Search for a free memory block with sufficient size
    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current->size < size) {
        prev = current;
        current = current->next;
    }

    // If no block with sufficient size found, return NULL
    if (current == NULL) return NULL;

    // If the block is too big, split it
    if (current->size > size + sizeof(block_t)) {
        block_t* new_block = (block_t*)((char*)current + size + sizeof(block_t));
        new_block->size = current->size - size - sizeof(block_t);
        new_block->next = current->next;
        current->size = size;
        current->next = new_block;
    }

    // Update free block
    if (prev == NULL) {
        free_list = current->next;
    } else {
        prev->next = current->next;
    }

    // Return pointer to the start of allocated block
    return (void*)(current + 1);
}

void my_free(void* ptr) {
    if (ptr == NULL) return;

    // Cast ptr to block_t pointer
    block_t* block = (block_t*)ptr - 1;

    // Check if ptr points to a valid block
    if ((char*)block < memory || (char*)block >= memory + MEMORY_SIZE) {
        fprintf(stderr, "Error: Attempt to free memory outside allocated memory\n");
        return;
    }

    // Check if the block is already freed
    if (block->size == 0) {
        fprintf(stderr, "Error: Attempt to free unallocated memory\n");
        return;
    }

    // Mark the block as free
    block->size = 0;

    // Merge with adjacent free blocks if possible
    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current < block) {
        prev = current;
        current = current->next;
    }

    // Merge with previous block if contiguous
    if (prev != NULL && (char*)prev + prev->size + sizeof(block_t) == (char*)block) {
        prev->size += block->size + sizeof(block_t);
        block = prev;
    } else {
        block->next = current;
        if (prev != NULL) {
            prev->next = block;
        } else {
            free_list = block;
        }
    }

    // Merge with next block if contiguous
    if (current != NULL && (char*)block + block->size + sizeof(block_t) == (char*)current) {
        block->size += current->size + sizeof(block_t);
        block->next = current->next;
    }
}



void* my_calloc(size_t nmemb, size_t size)
{
    (void)nmemb;
    (void)size;

    //Valid parametes check
    if (nmemb == 0 || size == 0) return NULL;

    //Calculate the total memory size to allocate
    size_t total_size = nmemb * size;

    //Check total memory size is not too big
    if (total_size / nmemb != size) return NULL; 

    //Alloc memory with my_alloc()
    void* ptr = my_malloc(total_size);
    if(ptr == NULL) return NULL;

    // Initilise memory allocated to 0 with memset()
    memset(ptr, 0, total_size);
    
    //Return
    return ptr;
}

void* my_realloc(void* ptr, size_t size) {
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    if (ptr == NULL) return my_malloc(size);

    // Cast ptr to block_t pointer
    block_t* block = (block_t*)ptr - 1;
    size_t old_size = block->size;

    // Allocate new memory block
    void* new_ptr = my_malloc(size);
    if (new_ptr == NULL) return NULL;

    // Copy data from old block to new block
    size_t copy_size = (old_size < size) ? old_size : size;
    memcpy(new_ptr, ptr, copy_size);

    // Free old block
    my_free(ptr);
    return new_ptr;
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
