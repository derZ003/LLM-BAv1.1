#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void mempcpy_recursive(char *s1, const char *s2, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return;
    }
    if (is_certain(_NOT_(n_zero))) {
        allocd(s1, 1);
        allocd((void *)s2, 1);
        *s1 = *s2;
        mempcpy_recursive(s1 + 1, s2 + 1, n - 1);
        return;
    }
    push_pc();
    assume(n_zero);
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    allocd(s1, 1);
    allocd((void *)s2, 1);
    *s1 = *s2;
    mempcpy_recursive(s1 + 1, s2 + 1, n - 1);
    pop_pc();
}

void *mempcpy(void *dest, const void *src, size_t n) {
    char *s1 = (char *)dest;
    const char *s2 = (const char *)src;
    mempcpy_recursive(s1, s2, n);
    return (void *)(s1 + n);
}
