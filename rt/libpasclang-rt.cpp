#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: recover stack map with LLVM, implement GC routines

static size_t __pasclang_heap_size;
static uint32_t __pasclang_gc_alloc_ptr;
static void* __pasclang_gc_from_space;
static void* __pasclang_gc_to_space;
static uint32_t __pasclang_gc_scan_ptr;

struct __pasclang_object {
    char flags;
    size_t size;
    void* object;
};

extern "C" {

// Sets the GC up. Must be called at the beginning of every Pseudo-Pascal
// program.
void __pasclang_gc_initialize(int32_t size);

// Allocates a GCed object. Return address is the data used by the program, not
// the structure.
void* __pasclang_gc_alloc(int32_t size, unsigned char type) {
    switch (type) {
    case 1: // Boolean
        return calloc(size, sizeof(char));
    case 2: // Integer
        return calloc(size, sizeof(int32_t));
    case 3: // Pointer
        return calloc(size, sizeof(char*));
    default:
        return NULL;
    }
}

// Triggers tri-color Cheney collection
void __pasclang_gc_collect();

// Copies object to the unused semi-space
__pasclang_object* __pasclang_gc_copy(__pasclang_object* object);

int readln() {
    int input;
    scanf("%d", &input);
    return input;
}

void write(int output) { printf("%d", output); }

void writeln(int output) { printf("%d\n", output); }

} // extern "C"
