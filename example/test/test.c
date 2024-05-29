#include <criterion/criterion.h>
#include <stdlib.h>
#include "my_secmalloc.private.h"

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


