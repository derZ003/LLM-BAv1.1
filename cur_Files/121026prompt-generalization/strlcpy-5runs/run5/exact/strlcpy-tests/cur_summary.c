#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlcpy_rec(char *dst, const char *src, size_t n, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)src;
    cnstr_t end = _EQ_(c, 0);

    cond_write(dst, c, guard);

    if (is_certain(end)) {
        return (symbolic)0;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    symbolic res;
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        res = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst, src + 1, n, g));
    } else if (is_certain(_NOT_(n_zero))) {
        res = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst + 1, src + 1, n - 1, g));
    } else {
        push_pc();
        assume(n_zero);
        symbolic a = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst, src + 1, n, _AND_(g, n_zero)));
        pop_pc();

        push_pc();
        assume(_NOT_(n_zero));
        symbolic b = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_rec(dst + 1, src + 1, n - 1, _AND_(g, _NOT_(n_zero))));
        pop_pc();

        res = _ITE_VAR_(n_zero, a, b);
    }
    pop_pc();

    return _ITE_VAR_(end, (symbolic)0, res);
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        // Library: if (!n) dst = dummy; while (*dst = *src != 0) { ... src++; } return src - src0;
        // We pass a pointer that doesn't affect the original dst buffer.
        return (size_t)(unsigned long)strlcpy_rec((char *)0, src, n, TRUE);
    }

    push_pc();
    assume(_NOT_(n_zero));
    // Library: --n; while...
    symbolic res = strlcpy_rec(dst, src, n - 1, _NOT_(n_zero));
    pop_pc();

    return (size_t)(unsigned long)res;
}
