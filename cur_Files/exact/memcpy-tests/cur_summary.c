#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void memcpy_recurse(unsigned char *p1, const unsigned char *p2, size_t n, cnstr_t guard) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return;
    }
    
    cnstr_t g = _AND_(guard, _NOT_(n_zero));
    symbolic val = *p2;
    cond_write(p1, val, g);
    
    push_pc();
    assume(_NOT_(n_zero));
    memcpy_recurse(p1 + 1, p2 + 1, n - 1, g);
    pop_pc();
}

void *memcpy(void *dest, const void *src, size_t n) {
    memcpy_recurse((unsigned char *)dest, (const unsigned char *)src, n, TRUE);
    return dest;
}
