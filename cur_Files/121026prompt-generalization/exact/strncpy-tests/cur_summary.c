#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strncpy_rec(char *d, const char *s, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cond_write(d, c, g);
    cnstr_t is_null = _EQ_(c, 0);
    if (is_certain(is_null)) {
        strncpy_rec(d + 1, s, n - 1, g);
    } else if (is_certain(_NOT_(is_null))) {
        strncpy_rec(d + 1, s + 1, n - 1, g);
    } else {
        push_pc();
        assume(is_null);
        strncpy_rec(d + 1, s, n - 1, _AND_(g, is_null));
        pop_pc();
        push_pc();
        assume(_NOT_(is_null));
        strncpy_rec(d + 1, s + 1, n - 1, _AND_(g, _NOT_(is_null)));
        pop_pc();
    }
    pop_pc();
}

char *strncpy(char *dest, const char *src, size_t n) {
    strncpy_rec(dest, src, n, TRUE);
    return dest;
}
