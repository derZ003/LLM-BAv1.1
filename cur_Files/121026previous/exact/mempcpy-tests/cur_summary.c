#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *mempcpy_rec(char *d, const char *s, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (void *)d;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic val = (symbolic)(unsigned char)*s;
    cond_write(d, val, g);
    symbolic r = mempcpy_rec(d + 1, s + 1, n - 1, g);
    pop_pc();
    return _ITE_VAR_(end, (void *)d, r);
}

void *mempcpy(void *dest, const void *src, size_t n) {
    return mempcpy_rec((char *)dest, (const char *)src, n, TRUE);
}
