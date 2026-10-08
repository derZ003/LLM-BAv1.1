#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void bzero_recurse(unsigned char *p, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return;
    }
    if (is_certain(_NOT_(n_zero))) {
        cond_write(p, 0, TRUE);
        bzero_recurse(p + 1, n - 1);
        return;
    }
    push_pc();
    assume(n_zero);
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    cond_write(p, 0, TRUE);
    bzero_recurse(p + 1, n - 1);
    pop_pc();
}

void bzero(void *s, size_t n) {
    bzero_recurse((unsigned char *)s, n);
}
