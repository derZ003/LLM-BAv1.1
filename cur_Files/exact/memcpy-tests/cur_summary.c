#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *memcpy_recursive(char *dest, const char *src, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return (void *)dest;
    }
    if (is_certain(_NOT_(n_zero))) {
        allocd(dest, 1);
        allocd((void *)src, 1);
        *dest = *src;
        return memcpy_recursive(dest + 1, src + 1, n - 1);
    }
    push_pc();
    assume(n_zero);
    void *res_zero = (void *)dest;
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    allocd(dest, 1);
    allocd((void *)src, 1);
    *dest = *src;
    void *res_step = memcpy_recursive(dest + 1, src + 1, n - 1);
    pop_pc();
    return (void *)_ITE_VAR_(n_zero, (symbolic)res_zero, (symbolic)res_step);
}

void *memcpy(void *dest, const void *src, size_t n) {
    // The function must return the original dest pointer.
    // To avoid the return value being corrupted by recursion in the symbolic engine,
    // we capture the original pointer.
    void *original_dest = dest;
    memcpy_recursive((char *)dest, (const char *)src, n);
    return original_dest;
}
