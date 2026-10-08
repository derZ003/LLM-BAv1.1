#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strnlen_rec(const char *s, size_t n) {
    cnstr_t end_n = _EQ_(n, 0);
    if (is_certain(end_n)) {
        return 0;
    }
    push_pc();
    assume(_NOT_(end_n));
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end_s = _EQ_(c, 0);
    symbolic r;
    if (is_certain(end_s)) {
        r = 0;
    } else if (is_certain(_NOT_(end_s))) {
        r = (symbolic)((unsigned long)1 + (unsigned long)strnlen_rec(s + 1, n - 1));
    } else {
        push_pc();
        assume(end_s);
        symbolic a = 0;
        pop_pc();
        push_pc();
        assume(_NOT_(end_s));
        symbolic b = (symbolic)((unsigned long)1 + (unsigned long)strnlen_rec(s + 1, n - 1));
        pop_pc();
        r = _ITE_VAR_(end_s, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end_n, (symbolic)0, r);
}

size_t strnlen(const char *s, size_t max) {
    return (size_t)(unsigned long)strnlen_rec(s, max);
}
