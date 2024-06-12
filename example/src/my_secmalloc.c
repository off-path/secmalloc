#define _GNU_SOURCE
#define CANARY 0xDEADBEEF
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

typedef struct block {
    size_t size;
    struct block* next;
    unsigned long canary_after; // Add canary after the block
} block_t;

block_t* free_list = NULL;
char* memory = NULL;
size_t memory_size = 0;

void* my_malloc(size_t size) {
    // Check for minimum size
    if (size == 0) {
        return NULL;
    }

    // Allocate memory if necessary
    if (memory == NULL) {
        memory_size = 10000;
        memory = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) {
            perror("mmap");
            return NULL;
        }

        // Initialize the free list
        free_list = (block_t*) memory;
        free_list->size = memory_size - sizeof(block_t);
        free_list->next = NULL;

        // Add canary after the block
        free_list->canary_after = CANARY;
    }

    // Search for a free block with sufficient size
    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current->size < size) {
        prev = current;
        current = current->next;
    }

    // If no block with sufficient size found, return NULL
    if (current == NULL) {
        return NULL;
    }

    // If the block is too big, divide it by two
    if (current->size > size + sizeof(block_t)) {
        block_t* new_block = (block_t*)((char*)current + size);
        new_block->size = current->size - size - sizeof(block_t);
        new_block->next = current->next;
        current->size = size;
        current->next = new_block;

        // Add canary after the new block
        new_block->canary_after = CANARY;
    }

    // Update the free block
    if (prev == NULL) {
        free_list = current->next;
    } else {
        prev->next = current->next;
    }

    // Add canary before the block
    void* ptr = (void*)(current + 1);
    *(unsigned long*)ptr = CANARY;
    ptr = (void*)((char*)ptr + sizeof(unsigned long));

    // Return a pointer to the start of the allocated block
    return ptr;
}

void my_free(void* ptr) {
    // Check if the pointer is valid
    if (ptr == NULL) {
        return;
    }

    // Retrieve the associated block
    block_t* block = (block_t*)ptr - 1;

    // Check if the block is valid
    if (block->size == 0) {
        return;
    }

    // Check if the pointer is within the allocated memory
    if ((char*)ptr < memory || (char*)ptr >= memory + memory_size) {
        return;
    }

    // Check canary before block
    if (*(unsigned long*)((char*)ptr - sizeof(unsigned long)) != CANARY) {
        printf("Memory corruption detected: canary before block is invalid\n");
        return;
    }

    // Check canary after block
    if (block->canary_after != CANARY) {
        printf("Memory corruption detected: canary after block is invalid\n");
        return;
    }

    // Add the block to the list of free blocks
    block->next = free_list;
    free_list = block;

    // Reset the allocated memory to avoid memory leaks
    memset(ptr, 0, block->size);

    // Check if the memory can be unmapped
    block_t* current = free_list;
    while (current != NULL) {
        if ((char*)(current + 1) < memory || (char*)(current + 1) >= memory + memory_size) {
            return;
        }
        current = current->next;
    }

    // Unmap the memory
    if (munmap(memory, memory_size) == -1) {
        perror("munmap");
    }
    memory = NULL;
    memory_size = 0;
}

void* my_calloc(size_t nmemb, size_t size) {
    // Validate parameters
    if (nmemb == 0 || size == 0) {
        return NULL;
    }

    // Calculate the total size
    size_t total_size = nmemb * size;

    // Check for overflow
    if (total_size / nmemb != size) {
        return NULL;
    }

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

    // Allocate a new block of memory using my_malloc
    void* new_ptr = my_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    // Copy the data from the old block to the new block
    size_t copy_size = (size_t)(size < block->size ? size : block->size);
    memcpy(new_ptr, ptr, copy_size);

    // Free the old block of memory using my_free
    my_free(ptr);

    // Return a pointer to the start of the reallocated block
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
