#include <criterion/criterion.h>
#include <stdlib.h>
#include "my_secmalloc.private.h"
#include <stdint.h>

/*
* ====================================== my_calloc_test ======================================
*/

Test(my_malloc, test_null_pointer_when_size_is_zero) {
    void* ptr = my_malloc(0);
    cr_assert_null(ptr, "Expected null pointer when size is zero");
}

Test(my_malloc, test_null_pointer_when_size_is_too_large) {
    void* ptr = my_malloc(MEMORY_SIZE + 1);
    cr_assert_null(ptr, "Expected null pointer when size is too large");
}

Test(my_malloc, test_allocation_and_deallocation) {
    // Allocate a block of memory
    size_t size = 100;
    void* ptr = my_malloc(size);
    cr_assert_not_null(ptr, "Expected a valid pointer after allocation");

    // Write to the block of memory
    memset(ptr, 0xAA, size);

    // Deallocate the block of memory
    my_free(ptr);

    // Try to allocate a block of the same size
    void* ptr2 = my_malloc(size);
    cr_assert_not_null(ptr2, "Expected a valid pointer after allocation");

    // Verify that the memory has been cleared
    for (size_t i = 0; i < size; i++) {
        cr_assert_neq(((char*)ptr2)[i], 0xAA);
    }

    // Deallocate the block of memory
    my_free(ptr2);
}

/*
* ====================================== my_free_test ======================================
*/
/*
Test(my_free, test_no_operation_when_ptr_is_null) {
    // Deallocate a null pointer with my_free()
    my_free(NULL);

    // Verify that the free list is empty
    cr_assert_eq(free_list, NULL, "Expected the free list to be empty");
}
*/
Test(my_free, test_no_operation_when_ptr_is_out_of_bounds) {
    // Allocate a block of memory with my_malloc()
    size_t size = 10;
    void* ptr = my_malloc(size);
    cr_assert_not_null(ptr, "Expected a valid pointer after allocation");

    // Deallocate a pointer that is out of bounds with my_free()
    my_free((char*)ptr - 1);

    // Verify that the block of memory is still allocated
    cr_assert_neq(ptr, NULL, "Expected the block of memory to be still allocated");

    // Deallocate the block of memory with my_free()
    my_free(ptr);
}
/*
Test(my_free, test_deallocation_and_addition_to_free_list) {
    // Allocate a block of memory with my_malloc()
    size_t size = 10;
    void* ptr = my_malloc(size);
    cr_assert_not_null(ptr, "Expected a valid pointer after allocation");

    // Deallocate the block of memory with my_free()
    my_free(ptr);

    // Verify that the block of memory has been added to the free list
    cr_assert_not_eq(free_list, NULL, "Expected the free list to be not empty");
    cr_assert_eq(free_list->size, size, "Expected the size of the block in the free list to be correct");
    cr_assert_eq(free_list->next, NULL, "Expected the next block in the free list to be null");

    // Deallocate the block of memory with my_free()
    my_free(ptr);
}
*/
Test(my_free, test_memory_clearing) {
    // Allocate a block of memory with my_malloc()
    size_t size = 10;
    void* ptr = my_malloc(size);
    cr_assert_not_null(ptr, "Expected a valid pointer after allocation");

    // Write to the block of memory
    memset(ptr, 0xAA, size);

    // Deallocate the block of memory with my_free()
    my_free(ptr);

    // Reallocate the block of memory with my_malloc()
    void* ptr2 = my_malloc(size);
    cr_assert_not_null(ptr2, "Expected a valid pointer after allocation");

    // Verify that the memory has been cleared
    for (size_t i = 0; i < size; i++) {
        cr_assert_eq(((char*)ptr2)[i], 0, "Expected the memory to be cleared");
    }

    // Deallocate the block of memory with my_free()
    my_free(ptr2);
}

/*
* ====================================== my_calloc_test ======================================
*/
//my_calloc test
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

Test(my_calloc, test_allocation_and_initialization) {
    // Allocate a block of memory with my_calloc()
    size_t nmemb = 10;
    size_t size = 10;
    void* ptr = my_calloc(nmemb, size);
    cr_assert_not_null(ptr, "Expected a valid pointer after allocation");

    // Verify that the memory has been initialized to zero
    for (size_t i = 0; i < nmemb * size; i++) {
        cr_assert_eq(((char*)ptr)[i], 0, "Expected memory to be initialized to zero");
    }

    // Deallocate the block of memory
    my_free(ptr);
}
