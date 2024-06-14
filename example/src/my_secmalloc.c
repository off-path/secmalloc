#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define MEMORY_SIZE 10000
#define CANARY_VALUE 0xDEADBEEF

typedef struct block {
    size_t size;
    size_t canary;
    struct block* next;
} block_t;

block_t* free_list = NULL;
char memory[MEMORY_SIZE];

static FILE *log_file = NULL;

void log_message(const char *format, ...);
void log_operation(const char *operation, size_t size, clock_t start, clock_t end);

void open_log_file() {
    if (log_file == NULL) {
        log_file = fopen("memory.log", "a");
        if (log_file == NULL) {
            log_message("Error opening log file");
        }
    }
}

void close_log_file() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_message(const char *format, ...) {
    open_log_file();
    if (log_file != NULL) {
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fprintf(log_file, "\n");
    }
}

void log_operation(const char *operation, size_t size, clock_t start, clock_t end) {
    log_message("%s: size=%zu, start=%ld, end=%ld", operation, size, start, end);
}

size_t generate_canary() {
    return CANARY_VALUE;
}

void insert_canary(block_t* block) {
    block->canary = CANARY_VALUE;
    size_t* end_canary = (size_t*)((char*)block + sizeof(block_t) + block->size);
    *end_canary = CANARY_VALUE;
}

int check_canary(block_t* block) {
    if (block->canary != CANARY_VALUE) {
        return 0; // Canary at the beginning of the block corrupted
    }
    size_t* end_canary = (size_t*)((char*)block + sizeof(block_t) + block->size);
    if (*end_canary != CANARY_VALUE) {
        return 0; // Canary at the end of the block corrupted
    }
    return 1; // Canaries valid
}

void* my_malloc(size_t size) {
    clock_t start = clock();

    if (size == 0 || size > MEMORY_SIZE - sizeof(block_t) - 2 * sizeof(size_t)) {
        log_operation("malloc", size, start, start); // Start and end are the same when returning early
        return NULL;
    }

    size_t total_size = size + sizeof(block_t) + 2 * sizeof(size_t);
    if (total_size > MEMORY_SIZE) {
        return NULL;
    }

    if (free_list == NULL) {
        free_list = (block_t*)memory;
        free_list->size = MEMORY_SIZE - sizeof(block_t) - 2 * sizeof(size_t);
        free_list->next = NULL;
    }

    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current->size < size) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return NULL;
    }

    if (current->size > total_size + sizeof(block_t) + 2 * sizeof(size_t)) {
        block_t* new_block = (block_t*)((char*)current + total_size);
        new_block->size = current->size - total_size;
        new_block->next = current->next;
        current->size = size;
        current->next = new_block;
    }

    if (prev == NULL) {
        free_list = current->next;
    } else {
        prev->next = current->next;
    }

    current->canary = CANARY_VALUE;
    insert_canary(current);

    void* user_ptr = (void*)((char*)current + sizeof(block_t) + sizeof(size_t));
    memset(user_ptr, 0, size);

    clock_t end = clock();
    log_operation("malloc", size, start, end);

    return user_ptr;
}

void my_free(void* ptr) {
    clock_t start_unused = clock();

    if (ptr == NULL) {
        return;
    }

    block_t* block = (block_t*)((char*)ptr - sizeof(size_t) - sizeof(block_t));

    if ((char*)block < memory || (char*)block >= memory + MEMORY_SIZE) {
        fprintf(stderr, "Error: Attempt to free memory outside allocated memory\n");
        return;
    }

    if (!check_canary(block)) {
        fprintf(stderr, "Error: Memory corruption detected (canary mismatch)\n");
        return;
    }

    block_t* current = free_list;
    block_t* prev = NULL;
    while (current != NULL && current < block) {
        prev = current;
        current = current->next;
    }

    if (prev != NULL && (char*)prev + prev->size + sizeof(block_t) + 2 * sizeof(size_t) == (char*)block) {
        prev->size += block->size + sizeof(block_t) + 2 * sizeof(size_t);
        block = prev;
    } else {
        block->next = current;
        if (prev != NULL) {
            prev->next = block;
        } else {
            free_list = block;
        }
    }

    if (current != NULL && (char*)block + block->size + sizeof(block_t) + 2 * sizeof(size_t) == (char*)current) {
        block->size += current->size + sizeof(block_t) + 2 * sizeof(size_t);
        block->next = current->next;
    }

    clock_t end_unused = clock();
    log_operation("free", 0, start_unused, end_unused);
}

void* my_calloc(size_t nmemb, size_t size) {
    clock_t start = clock();

    if (nmemb == 0 || size == 0) {
        return NULL;
    }

    size_t total_size = nmemb * size;
    if (total_size / nmemb != size) {
        return NULL;
    }

    void* ptr = my_malloc(total_size);
    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, total_size);

    clock_t end = clock();
    log_operation("calloc", total_size, start, end);

    return ptr;
}

void* my_realloc(void* ptr, size_t size) {
    clock_t start = clock();

    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return my_malloc(size);
    }

    block_t* block = (block_t*)((char*)ptr - sizeof(size_t) - sizeof(block_t));
    size_t old_size = block->size;

    if (old_size >= size) {
        clock_t end = clock();
        log_operation("realloc (no move)", size, start, end);
        return ptr;
    }

    void* new_ptr = my_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, old_size);
    my_free(ptr);

    clock_t end = clock();
    log_operation("realloc (move)", size, start, end);

    return new_ptr;
}

#ifdef DYNAMIC
void* malloc(size_t size) {
    return my_malloc(size);
}

void free(void* ptr) {
    my_free(ptr);
}

void* calloc(size_t nmemb, size_t size) {
    return my_calloc(nmemb, size);
}

void* realloc(void* ptr, size_t size) {
    return my_realloc(ptr, size);
}
#endif
