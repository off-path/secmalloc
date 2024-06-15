#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdlib.h>
#include "my_secmalloc.private.h"
#include <stdint.h>
#include <stdio.h>   // For fopen, fseek, ftell, fread, fclose
#include <string.h>  // For strstr
#include "my_secmalloc.private.h" 

#define SEEK_END 2
#define SEEK_SET 0
int check_canary(block_t* block);

/*
 * Helper function to read content from a log file
 */
char* read_log_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }
    fclose(file);
    return content;
}

/*
 * Test cases for my_malloc
 */
Test(my_malloc, test_null_pointer_when_size_is_zero) {
    void* ptr = my_malloc(0);
    cr_assert_null(ptr, "Expected null pointer when size is zero");
}

Test(my_malloc, test_null_pointer_when_size_is_too_large) {
    void* ptr = my_malloc(MEMORY_SIZE + 1);
    cr_assert_null(ptr, "Expected null pointer when size is too large");
}

Test(my_malloc, valid_allocation) {
    void* ptr = my_malloc(100);
    cr_assert_not_null(ptr, "my_malloc failed to allocate memory");

    char* log_content = read_log_file("memory.log");
    cr_assert_not_null(log_content, "Failed to read log file");

    cr_assert(strstr(log_content, "malloc: size=100") != NULL, "Log entry for my_malloc not found");
    free(log_content);

    my_free(ptr);
}

Test(my_malloc, zero_allocation) {
    void* ptr = my_malloc(0);
    cr_assert_null(ptr, "my_malloc(0) should return NULL");

    char* log_content = read_log_file("memory.log");
    cr_assert_not_null(log_content, "Failed to read log file");

    cr_assert(strstr(log_content, "malloc: size=0") != NULL, "Log entry for my_malloc(0) not found");
    free(log_content);
}

/*
 * Test cases for my_calloc
 */
Test(my_calloc, test_null_pointer_when_nmemb_is_zero) {
    void* ptr = my_calloc(0, 10);
    cr_assert_null(ptr, "Expected null pointer when nmemb is zero");
}

Test(my_calloc, test_null_pointer_when_size_is_zero) {
    void* ptr = my_calloc(10, 0);
    cr_assert_null(ptr, "Expected null pointer when size is zero");
}

Test(my_calloc, test_null_pointer_when_total_size_is_too_large) {
    void* ptr = my_calloc(SIZE_MAX, 1);
    cr_assert_null(ptr, "Expected null pointer when total size is too large");
}

Test(my_calloc, valid_allocation) {
    void* ptr = my_calloc(10, 10);
    cr_assert_not_null(ptr, "my_calloc failed to allocate memory");
    for (size_t i = 0; i < 100; ++i) {
        cr_assert_eq(((char*)ptr)[i], 0, "Memory not zero-initialized");
    }

    char* log_content = read_log_file("memory.log");
    cr_assert_not_null(log_content, "Failed to read log file");

    cr_assert(strstr(log_content, "calloc: size=100") != NULL, "Log entry for my_calloc not found");
    free(log_content);

    my_free(ptr);
}

/*
 * Test cases for my_realloc
 */
Test(my_realloc, test_null_ptr_zero_size) {
    void* ptr = my_realloc(NULL, 0);
    cr_assert_null(ptr, "Reallocation of NULL pointer with zero size should return NULL");
}

Test(my_realloc, test_non_null_ptr_zero_size) {
    int* ptr = malloc(sizeof(int));
    *ptr = 42;
    void* new_ptr = my_realloc(ptr, 0);
    cr_assert_null(new_ptr, "Reallocation of non-NULL pointer with zero size should free the memory");
    free(ptr); // Free memory allocated by malloc
}

Test(my_realloc, test_realloc_with_pointer_move) {
    int* ptr = malloc(sizeof(int));
    *ptr = 42;
    size_t new_size = 4 * sizeof(int);
    void* new_ptr = my_realloc(ptr, new_size);
    cr_assert_not_null(new_ptr, "Reallocation with pointer move should reallocate memory");
    int* new_int_ptr = (int*)new_ptr;
    cr_assert_eq(*new_int_ptr, 42, "Reallocated memory should preserve the original data");
    free(new_ptr);
}

/*
 * Test cases for my_free
 */
Test(my_free, valid_free) {
    void* ptr = my_malloc(100);
    cr_assert_not_null(ptr, "my_malloc failed to allocate memory");

    my_free(ptr);

    char* log_content = read_log_file("memory.log");
    cr_assert_not_null(log_content, "Failed to read log file");

    cr_assert(strstr(log_content, "free: size=0") != NULL, "Log entry for my_free not found");
    free(log_content);
}

/*
 * Test cases for heap overflow
 */
Test(my_malloc, heap_overflow) {
    size_t size = MEMORY_SIZE - sizeof(block_t) - 2 * sizeof(size_t);
    void* ptr = my_malloc(size);
    cr_assert_not_null(ptr, "my_malloc failed to allocate maximum allowable memory");

    char* overflow_ptr = (char*)ptr + size;
    *overflow_ptr = 'A';  // Devrait corrompre la mémoire
    cr_expect_eq(*(char*)((char*)ptr + size), 'A', "Heap overflow detected");

    my_free(ptr);
}

/*
 * Test cases for double free
 */
Test(my_free, double_free) {
    void* ptr = my_malloc(100);
    cr_assert_not_null(ptr, "my_malloc failed to allocate memory");

    my_free(ptr);

    // Double free should be handled
    FILE *stderr_backup = stderr;
    stderr = fopen("/dev/null", "w");
    my_free(ptr);
    fclose(stderr);
    stderr = stderr_backup;

    char* log_content = read_log_file("memory.log");
    cr_assert_not_null(log_content, "Failed to read log file");

    cr_assert(strstr(log_content, "free: size=0") != NULL, "Log entry for my_free not found");
    free(log_content);
}

/*
 * Test cases for memory leak
 */
Test(memory_leak, no_leak) {
    void* ptr1 = my_malloc(100);
    cr_assert_not_null(ptr1, "my_malloc failed to allocate memory");
    void* ptr2 = my_malloc(200);
    cr_assert_not_null(ptr2, "my_malloc failed to allocate memory");

    my_free(ptr1);
    my_free(ptr2);

    // Vérification que la liste libre contient tous les blocs
    block_t* current = free_list;
    size_t total_free_size = 0;
    while (current != NULL) {
        total_free_size += current->size + sizeof(block_t) + 2 * sizeof(size_t);
        current = current->next;
    }
    cr_assert_eq(total_free_size, MEMORY_SIZE, "Memory leak detected, total free size does not match");
}

/*
 * Test cases for heap underflow
 */
Test(my_malloc, heap_underflow) {
    void* ptr = my_malloc(100);
    cr_assert_not_null(ptr, "my_malloc failed to allocate memory");

    char* underflow_ptr = (char*)ptr - 1;
    *underflow_ptr = 'A';  // This should not corrupt the memory
    cr_expect_eq(*(char*)((char*)ptr - 1), 'A', "Heap underflow detected");

    my_free(ptr);
}

/*
*   TEST super big MALLOC
*/
Test(my_malloc, test_null_pointer_when_size_exceeds_memory_size) {
    void* ptr = my_malloc(1500000);
    cr_assert_null(ptr, "Expected null pointer when size exceeds MEMORY_SIZE");
}

/*
* TEST super small MALLOC
*/
Test(my_malloc, minimal_allocation) {
    void* ptr = my_malloc(1);
    cr_assert_not_null(ptr, "my_malloc failed to allocate minimal memory");
    my_free(ptr);
}

/*
* TEST Multi allocation
*/
Test(my_malloc, multiple_allocations) {
    void* ptr1 = my_malloc(100);
    void* ptr2 = my_malloc(200);
    void* ptr3 = my_malloc(300);
    cr_assert_not_null(ptr1, "my_malloc failed to allocate memory for ptr1");
    cr_assert_not_null(ptr2, "my_malloc failed to allocate memory for ptr2");
    cr_assert_not_null(ptr3, "my_malloc failed to allocate memory for ptr3");

    my_free(ptr1);
    my_free(ptr2);
    my_free(ptr3);
}

/*
* TEST Calloc zero ellements
*/
Test(my_calloc, zero_elements) {
    void* ptr = my_calloc(0, 100);
    cr_assert_null(ptr, "my_calloc(0, 100) should return NULL");
}

/*
* BIG Calloc
*/
Test(my_calloc, large_allocation) {
    void* ptr = my_calloc(1000, 10);
    cr_assert_not_null(ptr, "my_calloc failed to allocate memory");
    for (size_t i = 0; i < 10000; ++i) {
        cr_assert_eq(((char*)ptr)[i], 0, "Memory not zero-initialized");
    }
    my_free(ptr);
}

// Mini realloc
Test(my_realloc, shrink_allocation) {
    int* ptr = my_malloc(10 * sizeof(int));
    for (int i = 0; i < 10; ++i) ptr[i] = i;

    int* new_ptr = my_realloc(ptr, 5 * sizeof(int));
    cr_assert_not_null(new_ptr, "my_realloc failed to reallocate memory to smaller size");
    for (int i = 0; i < 5; ++i) {
        cr_assert_eq(new_ptr[i], i, "Data not preserved during reallocation");
    }
    my_free(new_ptr);
}

// bigger realloc
Test(my_realloc, expand_allocation) {
    int* ptr = my_malloc(5 * sizeof(int));
    for (int i = 0; i < 5; ++i) ptr[i] = i;

    int* new_ptr = my_realloc(ptr, 10 * sizeof(int));
    cr_assert_not_null(new_ptr, "my_realloc failed to reallocate memory to larger size");
    for (int i = 0; i < 5; ++i) {
        cr_assert_eq(new_ptr[i], i, "Data not preserved during reallocation");
    }
    my_free(new_ptr);
}

//free null ptr
Test(my_free, free_null_pointer) {
    my_free(NULL);
    // Test pas de crash
}

// Canary test Corruption
Test(my_malloc, canary_corruption) {
    void* ptr = my_malloc(100);
    cr_assert_not_null(ptr, "my_malloc failed to allocate memory");

    // Corrupt the canary
    block_t* block = (block_t*)((char*)ptr - sizeof(size_t) - sizeof(block_t));
    block->canary = 0xBADF00D;

    FILE *stderr_backup = stderr;
    stderr = fopen("/dev/null", "w");
    my_free(ptr);
    fclose(stderr);
    stderr = stderr_backup;
}

//Fragmentation memory test
Test(memory_management, fragmentation) {
    void* ptr1 = my_malloc(100);
    void* ptr2 = my_malloc(200);
    void* ptr3 = my_malloc(300);
    cr_assert_not_null(ptr1, "my_malloc failed to allocate memory for ptr1");
    cr_assert_not_null(ptr2, "my_malloc failed to allocate memory for ptr2");
    cr_assert_not_null(ptr3, "my_malloc failed to allocate memory for ptr3");

    my_free(ptr2);
    void* ptr4 = my_malloc(150);
    cr_assert_not_null(ptr4, "my_malloc failed to allocate memory for ptr4 after fragmentation");
    my_free(ptr1);
    my_free(ptr3);
    my_free(ptr4);
}

//Test en dessous de la limite maximale
Test(limits, near_max_allocation) {
    void* ptr = my_malloc(MEMORY_SIZE - sizeof(block_t) - 2 * sizeof(size_t));
    cr_assert_not_null(ptr, "my_malloc failed to allocate near max memory");
    my_free(ptr);
}

//Test canary
Test(canary, canary_check_after_multiple_operations) {
    void* ptr1 = my_malloc(100);
    void* ptr2 = my_malloc(200);
    void* ptr3 = my_malloc(300);
    cr_assert_not_null(ptr1, "my_malloc failed to allocate memory for ptr1");
    cr_assert_not_null(ptr2, "my_malloc failed to allocate memory for ptr2");
    cr_assert_not_null(ptr3, "my_malloc failed to allocate memory for ptr3");

    my_free(ptr2);
    my_free(ptr1);
    my_free(ptr3);

    block_t* block = free_list;
    while (block != NULL) {
        cr_assert(check_canary(block), "Canary check failed after multiple operations");
        block = block->next;
    }
}
