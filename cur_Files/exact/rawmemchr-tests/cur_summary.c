#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *rawmemchr_rec(const unsigned char *s, unsigned char c) {
    cnstr_t found = _EQ_(*s, c);
    if (is_certain(found)) {
        return (void *)s;
    } else if (is_certain(_NOT_(found))) {
        return rawmemchr_rec(s + 1, c);
    } else {
        push_pc();
        assume(found);
        void *a = (void *)s;
        pop_pc();
        push_pc();
        assume(_NOT_(found));
        void *b = rawmemchr_rec(s + 1, c);
        pop_pc();
        return _ITE_VAR_(found, a, b);
    }
}

void *rawmemchr(const void *s, int c) {
    return rawmemchr_rec((const unsigned char *)s, (unsigned char)c);
}
