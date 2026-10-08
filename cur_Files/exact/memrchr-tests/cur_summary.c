#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memrchr_rec(const unsigned char *s, unsigned char c, size_t n, symbolic current_ptr) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));

    symbolic val = (symbolic)(unsigned long)*(const unsigned char *)current_ptr;
    cnstr_t found = _EQ_(val, (symbolic)c);

    symbolic r;
    if (is_certain(found)) {
        r = current_ptr;
    } else if (is_certain(_NOT_(found))) {
        r = memrchr_rec(s, c, n - 1, (symbolic)((unsigned long)current_ptr - 1));
    } else {
        push_pc();
        assume(found);
        symbolic a = current_ptr;
        pop_pc();
        push_pc();
        assume(_NOT_(found));
        symbolic b = memrchr_rec(s, c, n - 1, (symbolic)((unsigned long)current_ptr - 1));
        pop_pc();
        r = _ITE_VAR_(found, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

void *memrchr(const void *s, int c, size_t n) {
    const unsigned char *start = (const unsigned char *)s;
    symbolic end_ptr = (symbolic)((unsigned long)start + (unsigned long)n - 1);
    
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return NULL;
    }
    
    symbolic res = memrchr_rec(start, (unsigned char)c, n, end_ptr);
    
    cnstr_t res_null = _EQ_(res, (symbolic)0);
    if (is_certain(res_null)) {
        return NULL;
    } else if (is_certain(_NOT_(res_null))) {
        return (void *)res;
    } else {
        push_pc();
        assume(res_null);
        symbolic a = (symbolic)0;
        pop_pc();
        push_pc();
        assume(_NOT_(res_null));
        symbolic b = res;
        pop_pc();
        return (void *)_ITE_VAR_(res_null, a, b);
    }
}
