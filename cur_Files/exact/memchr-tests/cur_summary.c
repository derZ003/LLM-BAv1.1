#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *memchr_rec(const char *s, int c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return (void *)0;
    }
    if (is_certain(_NOT_(n_zero))) {
        symbolic val = *(const char *)s;
        cnstr_t found = _EQ_(val, (char)c);
        if (is_certain(found)) {
            return (void *)s;
        }
        if (is_certain(_NOT_(found))) {
            return memchr_rec(s + 1, c, n - 1);
        }
        push_pc();
        assume(found);
        void *res_found = (void *)s;
        pop_pc();
        push_pc();
        assume(_NOT_(found));
        void *res_not_found = memchr_rec(s + 1, c, n - 1);
        pop_pc();
        return _ITE_VAR_(found, (symbolic)res_found, (symbolic)res_not_found);
    }
    push_pc();
    assume(n_zero);
    void *res_zero = (void *)0;
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    void *res_nonzero = memchr_rec(s, c, n); // This call is handled by the logic above, but for structure:
    // To avoid infinite recursion in the 'undecided' case, we must call the recursive step with n-1 or use the logic above.
    // Redoing the undecided case properly:
    pop_pc(); 
    
    // Correct logic for undecided n_zero:
    push_pc();
    assume(n_zero);
    void *r0 = (void *)0;
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    // Inside here, n is not 0, so we perform the check and recurse
    symbolic v = *(const char *)s;
    cnstr_t f = _EQ_(v, (char)c);
    push_pc();
    assume(f);
    void *rf = (void *)s;
    pop_pc();
    push_pc();
    assume(_NOT_(f));
    void *rnf = memchr_rec(s + 1, c, n - 1);
    pop_pc();
    void *r_nonzero = _ITE_VAR_(f, (symbolic)rf, (symbolic)rnf);
    pop_pc();
    
    return _ITE_VAR_(n_zero, (symbolic)r0, (symbolic)r_nonzero);
}

// Since the above helper had a structure error in the "undecided" block, 
// here is the clean, exact version following the API guidelines.

static void *memchr_impl(const char *s, int c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return (void *)0;
    }
    
    // Case where n is definitely not zero
    if (is_certain(_NOT_(n_zero))) {
        symbolic val = *(const char *)s;
        cnstr_t found = _EQ_(val, (char)c);
        if (is_certain(found)) {
            return (void *)s;
        }
        if (is_certain(_NOT_(found))) {
            return memchr_impl(s + 1, c, n - 1);
        }
        push_pc(); assume(found); void *a = (void *)s; pop_pc();
        push_pc(); assume(_NOT_(found)); void *b = memchr_impl(s + 1, c, n - 1); pop_pc();
        return _ITE_VAR_(found, (symbolic)a, (symbolic)b);
    }

    // Undecided n_zero
    push_pc();
    assume(n_zero);
    void *r_zero = (void *)0;
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    // Now n is certainly not zero, we can safely call the logic for n != 0
    // To avoid infinite recursion, we must manually implement the step here
    symbolic val = *(const char *)s;
    cnstr_t found = _EQ_(val, (char)c);
    push_pc(); assume(found); void *rf = (void *)s; pop_pc();
    push_pc(); assume(_NOT_(found)); void *rnf = memchr_impl(s + 1, c, n - 1); pop_pc();
    void *r_not_zero = _ITE_VAR_(found, (symbolic)rf, (symbolic)rnf);
    pop_pc();
    return _ITE_VAR_(n_zero, (symbolic)r_zero, (symbolic)r_not_zero);
}

void *memchr(const void *s, int c, size_t n) {
    return memchr_impl((const char *)s, c, n);
}
