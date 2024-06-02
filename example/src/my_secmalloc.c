#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

typedef struct block {
    size_t size;
    struct block* next;
} block_t;

block_t* free_list = NULL;

void* my_malloc(size_t size) {
    // Check for minimum size
    if (size == 0) {
        return NULL;
    }

    // Allocate memory using mmap
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    // Create a new block and initialize it
    block_t* new_block = (block_t*)ptr;
    new_block->size = size;
    new_block->next = NULL;

    // Update the free list
    free_list = new_block;

    // Return a pointer to the start of the allocated block
    return (void*)(new_block + 1);
}

void my_free(void* ptr) {
    // Check if the pointer is valid
    if (ptr == NULL) {
        return;
    }

    // Retrieve the associated block
    block_t* block = (block_t*)ptr - 1;

    // Remove the block from the free list
    if (free_list == block) {
        free_list = block->next;
    } else {
        block_t* current = free_list;
        while (current->next != block) {
            current = current->next;
        }
        current->next = block->next;
    }

    // Deallocate memory using munmap
    if (munmap(block, block->size) == -1) {
        perror("munmap");
    }
}

void* my_calloc(size_t nmemb, size_t size) {
    // Validate parameters
    if (nmemb == 0 || size == 0) {
        return NULL;
    }

    // Calculate the total size
    size_t total_size = nmemb * size;

    // Allocate memory using my_malloc
    void* ptr = my_malloc(total_size);
    if (ptr == NULL) {
        return NULL;
    }

    // Initialize the memory to zero
    memset(ptr, 0, total_size);

    // Return the pointer
    return ptr;
}

void* my_realloc(void* ptr, size_t size) {
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return my_malloc(size);
    }

    // Retrieve the associated block
    block_t* block = (block_t*)ptr - 1;

    // Remove the block from the free list
    if (free_list == block) {
        free_list = block->next;
    } else {
        block_t* current = free_list;
        while (current->next != block) {
            current = current->next;
        }
        current->next = block->next;
    }

    // Reallocate memory using mremap
    void* new_ptr = mremap(block, block->size, size, MREMAP_MAYMOVE);
    if (new_ptr == MAP_FAILED) {
        perror("mremap");
        // Re-add the block to the free list
        block->next = free_list;
        free_list = block;
        return NULL;
    }

    // Update the block size
    block_t* new_block = (block_t*)new_ptr;
    new_block->size = size;

    // Add the block to the free list
    new_block->next = free_list;
    free_list = new_block;

    // Return a pointer to the start of the reallocated block
    return (void*)(new_block + 1);
}

