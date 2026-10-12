#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic stpncpy_rec(char *s1, const char *s2, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s2;
    cond_write(s1, c, g);
    cnstr_t is_null = _EQ_(c, 0);
    symbolic res;
    if (is_certain(is_null)) {
        symbolic r = stpncpy_rec(s1 + 1, s2, n - 1, _AND_(g, is_null));
        res = r;
    } else if (is_certain(_NOT_(is_null))) {
        symbolic r = stpncpy_rec(s1 + 1, s2 + 1, n - 1, _AND_(g, _NOT_(is_null)));
        res = (symbolic)((unsigned long)r + 1);
    } else {
        push_pc();
        assume(is_null);
        symbolic r_null = stpncpy_rec(s1 + 1, s2, n - 1, _AND_(g, is_null));
        symbolic v_null = r_null;
        pop_pc();
        push_pc();
        assume(_NOT_(is_null));
        symbolic r_not_null = stpncpy_rec(s1 + 1, s2 + 1, n - 1, _AND_(g, _NOT_(is_null)));
        symbolic v_not_null = (symbolic)((unsigned long)r_not_null + 1);
        pop_pc();
        res = _ITE_VAR_(is_null, v_null, v_not_null);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, res);
}

char *stpncpy(char *s1, const char *s2, size_t n) {
    symbolic offset = stpncpy_rec(s1, s2, n, TRUE);
    return (char *)((unsigned long)s1 + (unsigned long)offset);
}
