#include <criterion/criterion.h>
#include <stdio.h>
#include "my_secmalloc.private.h"
#include <sys/mman.h>

Test(mmap, simple) {
    void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    cr_expect(ptr != NULL);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);
}


// Test for my_secmalloc


// 1: Allouer un bloc de mémoire standard.

Test(secmalloc_tests, malloc_nonzero_size) {
    int* ptr = my_malloc(10 * sizeof(int));
    cr_assert_not_null(ptr, "malloc returned NULL for a valid size request");
    my_free(ptr);
}

// 2: Demander zéro octet et observer le comportement.

Test(secmalloc_tests, malloc_zero_size) {
    int* ptr = my_malloc(0);
    cr_assert_null(ptr, "malloc should return NULL when asked for zero size");
}

// 3: Allouer une quantité très grande de mémoire.

Test(secmalloc_tests, malloc_large_size) {
    int* ptr = my_malloc(100 * 1024 * 1024); // Allouer environ 100 Mo
    cr_assert_not_null(ptr, "malloc should not return NULL for a large but reasonable size request");
    my_free(ptr);
}




// Test for my_free

// 1: Libérer un bloc de mémoire précédemment alloué.

Test(secmalloc_tests, free_valid_pointer) {
    int* ptr = my_malloc(10 * sizeof(int));
    my_free(ptr);
    cr_expect(true, "No crash on freeing a valid pointer");
}

// 2: Tenter de libérer un pointeur NULL.

Test(secmalloc_tests, free_null_pointer) {
    my_free(NULL);
    cr_expect(true, "No crash on freeing NULL pointer");
}

// 3: Libérer deux fois le même bloc (ce qui devrait être évité ou géré de manière sécurisée).

Test(secmalloc_tests, free_same_pointer_twice) {
    int* ptr = my_malloc(10 * sizeof(int));
    my_free(ptr);
    my_free(ptr);
    cr_expect(true, "No crash on freeing the same pointer twice");
}




//  Test for my_calloc

// 1: Allouer et initialiser un bloc de mémoire.

Test(secmalloc_tests, calloc_standard_usage) {
    int* ptr = my_calloc(10, sizeof(int));
    bool all_zero = true;
    for (int i = 0; i < 10; i++) {
        if (ptr[i] != 0) {
            all_zero = false;
            break;
        }
    }
    cr_assert(all_zero, "calloc should initialize memory to zero");
    my_free(ptr);
}

// 2: Allouer zéro élément et zéro taille, vérifier le comportement.

Test(secmalloc_tests, calloc_zero_elements_size) {
    int* ptr = my_calloc(0, 0);
    cr_assert_null(ptr, "calloc should return NULL when asked to allocate zero elements of zero size");
}


//  Tes for my_realloc

// 1: Augmenter la taille d'un bloc existant.

Test(secmalloc_tests, realloc_increase_size) {
    int* ptr = my_malloc(10 * sizeof(int));
    int* new_ptr = my_realloc(ptr, 20 * sizeof(int));
    cr_assert_not_null(new_ptr, "realloc should return a non-NULL pointer when increasing size");
    my_free(new_ptr);
}

// 2: Diminuer la taille d'un bloc existant.

Test(secmalloc_tests, realloc_decrease_size) {
    int* ptr = my_malloc(10 * sizeof(int));
    int* new_ptr = my_realloc(ptr, 5 * sizeof(int));
    cr_assert_not_null(new_ptr, "realloc should return a non-NULL pointer when decreasing size");
    my_free(new_ptr);
}

// 3: Réallouer un pointeur NULL (devrait se comporter comme malloc).

Test(secmalloc_tests, realloc_null_pointer) {
    int* ptr = my_realloc(NULL, 10 * sizeof(int));
    cr_assert_not_null(ptr, "realloc with NULL pointer should behave like malloc");
    my_free(ptr);
}