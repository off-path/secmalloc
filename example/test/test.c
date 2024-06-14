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
