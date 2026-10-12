#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memcmp_rec(const char *s1, const char *s2, size_t n) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return 0;
    }
    push_pc();
    assume(_NOT_(end));

    symbolic b1 = (symbolic)(unsigned char)*s1;
    symbolic b2 = (symbolic)(unsigned char)*s2;
    cnstr_t eq = _EQ_(b1, b2);

    symbolic r;
    if (is_certain(eq)) {
        r = memcmp_rec(s1 + 1, s2 + 1, n - 1);
    } else if (is_certain(_NOT_(eq))) {
        symbolic diff = (symbolic)((int)(unsigned char)*s1) - (int)(unsigned char)*s2;
        r = diff;
    } else {
        push_pc();
        assume(eq);
        symbolic a = memcmp_rec(s1 + 1, s2 + 1, n - 1);
        pop_pc();

        push_pc();
        assume(_NOT_(eq));
        symbolic b_res = (symbolic)((int)(unsigned char)*s1) - (int)(unsigned char)*s2;
        pop_pc();
        r = _ITE_VAR_(eq, a, b_res);
    }

    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

int memcmp(const void *s1, const void *s2, size_t n) {
    return (int)memcmp_rec((const char *)s1, (const char *)s2, n);
}
