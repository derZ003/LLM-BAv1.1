typedef unsigned int size_t;

__attribute__((weak)) void* mem_alloc(size_t bytes) {return 0;}
__attribute__((weak)) void mem_free(void* ptr) {return;}

void* malloc(size_t size) {
    return mem_alloc(size);
}

void free(void* ptr) {
    mem_free(ptr);
}
