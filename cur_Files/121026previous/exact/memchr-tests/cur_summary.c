#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memchr_rec(const char *s, symbolic c, size_t n) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));
    symbolic cur = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t found = _EQ_(cur, c);
    symbolic r;
    if (is_certain(found)) {
        r = (symbolic)s;
    } else if (is_certain(_NOT_(found))) {
        r = memchr_rec(s + 1, c, n - 1);
    } else {
        push_pc();
        assume(found);
        symbolic a = (symbolic)s;
        pop_pc();
        push_pc();
        assume(_NOT_(found));
        symbolic b = memchr_rec(s + 1, c, n - 1);
        pop_pc();
        r = _ITE_VAR_(found, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

void *memchr(const void *s, int c, size_t n) {
    return (void *)memchr_rec((const char *)s, (symbolic)(unsigned char)c, n);
}
