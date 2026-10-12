#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memchr_rec(const char *s, symbolic c, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic cur = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t match = _EQ_(cur, c);
    symbolic r;
    if (is_certain(match)) {
        r = (symbolic)s;
    } else if (is_certain(_NOT_(match))) {
        r = memchr_rec(s + 1, c, n - 1, g);
    } else {
        push_pc();
        assume(match);
        symbolic a = (symbolic)s;
        pop_pc();
        push_pc();
        assume(_NOT_(match));
        symbolic b = memchr_rec(s + 1, c, n - 1, _AND_(g, _NOT_(match)));
        pop_pc();
        r = _ITE_VAR_(match, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

void *memchr(const void *s, int c, size_t n) {
    symbolic sc = (symbolic)(unsigned char)c;
    symbolic res = memchr_rec((const char *)s, sc, n, TRUE);
    return (void *)res;
}
