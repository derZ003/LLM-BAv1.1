#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlcpy_rec(char *dst, const char *src, size_t n, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)src;
    cnstr_t is_null = _EQ_(c, 0);

    cond_write(dst, c, guard);

    if (is_certain(is_null)) {
        return (symbolic)0;
    }

    push_pc();
    assume(_NOT_(is_null));
    cnstr_t g = _AND_(guard, _NOT_(is_null));

    cnstr_t n_zero = _EQ_(n, 0);
    symbolic r;
    if (is_certain(n_zero)) {
        r = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst, src + 1, n, g));
    } else if (is_certain(_NOT_(n_zero))) {
        r = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst + 1, src + 1, n - 1, g));
    } else {
        push_pc();
        assume(n_zero);
        symbolic a = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst, src + 1, n, _AND_(g, n_zero)));
        pop_pc();

        push_pc();
        assume(_NOT_(n_zero));
        symbolic b = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst + 1, src + 1, n - 1, _AND_(g, _NOT_(n_zero))));
        pop_pc();

        r = _ITE_VAR_(n_zero, a, b);
    }
    pop_pc();

    return _ITE_VAR_(is_null, (symbolic)0, r);
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return (size_t)(unsigned long)strlcpy_rec((char *)0, src, n, FALSE);
    }
    if (is_certain(_NOT_(n_zero))) {
        return (size_t)(unsigned long)strlcpy_rec(dst, src, n - 1, TRUE);
    }
    push_pc();
    assume(n_zero);
    symbolic a = strlcpy_rec((char *)0, src, n, _AND_(TRUE, n_zero));
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    symbolic b = strlcpy_rec(dst, src, n - 1, _AND_(TRUE, _NOT_(n_zero)));
    pop_pc();
    return (size_t)(unsigned long)_ITE_VAR_(n_zero, a, b);
}
