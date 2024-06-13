#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "my_secmalloc.h"

// Test d'allocation et de désallocation
void test_alloc_dealloc() {
    printf("Exécution du test_alloc_dealloc...\n");
    void *ptr1 = malloc(16);
    assert(ptr1 != NULL);
    void *ptr2 = malloc(32);
    assert(ptr2 != NULL);
    free(ptr1);
    free(ptr2);
    printf("Test réussi \n");
}

// Test de calloc
void test_calloc() {
    printf("Exécution du test_calloc...\n");
    void *ptr = calloc(4, 8);
    assert(ptr != NULL);
    for (size_t i = 0; i < 32; i++) {
        assert(((char *)ptr)[i] == 0);
    }
    free(ptr);
    printf("Test réussi : calloc\n");
}

// Test de realloc
void test_realloc() {
    printf("Exécution du test_realloc...\n");
    void *ptr = malloc(16);
    assert(ptr != NULL);
    ptr = realloc(ptr, 32);
    assert(ptr != NULL);
    ptr = realloc(ptr, 8);
    assert(ptr != NULL);
    free(ptr);
    printf("Test réussi : realloc\n");
}

// Test mémoire 
void test_memory_leak() {
    printf("Exécution du test_memory_leak...\n");

// Allocation de mémoire
    void *ptr = malloc(16);
    assert(ptr != NULL);
    printf("Test réussi : RIP la mémoire \n");
}

// // Test
// void test_functional() {
//     printf("Exécution du test_functional...\n");
//     char* x = malloc(10000);
//     *x = 1;
//     printf("GOOD\n");
//     free(x);
//     *x = 1; 
//     printf(" PWN !\n");
// }

int main() {
    test_alloc_dealloc();
    test_calloc();
    test_realloc();
    test_memory_leak();
    test_functional();
    printf("Tous les tests ont réussi avec succès !\n");
    return 0;
}