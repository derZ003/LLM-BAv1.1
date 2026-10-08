#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *memrchr_rec(const unsigned char *s, unsigned char c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return (void *)0;
    }
    if (is_certain(_NOT_(n_zero))) {
        const unsigned char *curr = s + n - 1;
        cnstr_t match = _EQ_(*curr, c);
        if (is_certain(match)) {
            return (void *)curr;
        }
        if (is_certain(_NOT_(match))) {
            return memrchr_rec(s, c, n - 1);
        }
        push_pc();
        assume(match);
        void *a = (void *)curr;
        pop_pc();
        push_pc();
        assume(_NOT_(match));
        void *b = memrchr_rec(s, c, n - 1);
        pop_pc();
        return _ITE_VAR_(match, a, b);
    }
    push_pc();
    assume(_NOT_(n_zero));
    void *res_nonzero = memrchr_rec(s, c, n - 1); // This is wrong for memrchr logic; it needs the actual loop behavior
    // Correcting: if n != 0, we check the last element and then recurse.
    pop_pc();
    // To maintain exactness and termination, we must handle n_zero first.
    // Re-evaluating the step:
    push_pc();
    assume(_NOT_(n_zero));
    const unsigned char *curr_sym = s + n - 1;
    cnstr_t match_sym = _EQ_(*curr_sym, c);
    push_pc();
    assume(match_sym);
    void *match_val = (void *)curr_sym;
    pop_pc();
    push_pc();
    assume(_NOT_(match_sym));
    void *no_match_val = memrchr_rec(s, c, n - 1);
    pop_pc();
    symbolic res_step = _ITE_VAR_(match_sym, match_val, no_match_val);
    pop_pc();
    
    return _ITE_VAR_(n_zero, (symbolic)(void *)0, res_step);
}

// The recursive helper needs to return symbolic to use _ITE_VAR_
static symbolic memrchr_helper(const unsigned char *s, unsigned char c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) return (symbolic)(void *)0;
    if (is_certain(_NOT_(n_zero))) {
        const unsigned char *curr = s + n - 1;
        cnstr_t match = _EQ_(*curr, c);
        if (is_certain(match)) return (symbolic)(void *)curr;
        if (is_certain(_NOT_(match))) return memrchr_helper(s, c, n - 1);
        push_pc(); assume(match); symbolic a = (symbolic)(void *)curr; pop_pc();
        push_pc(); assume(_NOT_(match)); symbolic b = memrchr_helper(s, c, n - 1); pop_pc();
        return _ITE_VAR_(match, a, b);
    }
    push_pc(); assume(_NOT_(n_zero)); 
    const unsigned char *curr_sym = s + n - 1;
    cnstr_t match_sym = _EQ_(*curr_sym, c);
    push_pc(); assume(match_sym); symbolic a_sym = (symbolic)(void *)curr_sym; pop_pc();
    push_pc(); assume(_NOT_(match_sym)); symbolic b_sym = memrchr_helper(s, c, n - 1); pop_pc();
    symbolic res_nonzero = _ITE_VAR_(match_sym, a_sym, b_sym);
    pop_pc();
    return _ITE_VAR_(n_zero, (symbolic)(void *)0, res_nonzero);
}

void *memrchr(const void *s, int c, size_t n) {
    return (void *)memrchr_helper((const unsigned char *)s, (unsigned char)c, n);
}
