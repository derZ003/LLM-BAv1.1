#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strncpy_rec(char *s1, const char *s2, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s2;
    cnstr_t is_zero = _EQ_(c, 0);

    if (is_certain(is_zero)) {
        cond_write(s1, 0, g);
        strncpy_rec(s1 + 1, s2, n - 1, g);
    } else if (is_certain(_NOT_(is_zero))) {
        cond_write(s1, c, g);
        strncpy_rec(s1 + 1, s2 + 1, n - 1, g);
    } else {
        push_pc();
        assume(is_zero);
        cond_write(s1, 0, _AND_(g, is_zero));
        strncpy_rec(s1 + 1, s2, n - 1, _AND_(g, is_zero));
        pop_pc();

        push_pc();
        assume(_NOT_(is_zero));
        cond_write(s1, c, _AND_(g, _NOT_(is_zero)));
        strncpy_rec(s1 + 1, s2 + 1, n - 1, _AND_(g, _NOT_(is_zero)));
        pop_pc();
    }
    pop_pc();
}

char *strncpy(char *dest, const char *src, size_t n) {
    strncpy_rec(dest, src, n, TRUE);
    return dest;
}
