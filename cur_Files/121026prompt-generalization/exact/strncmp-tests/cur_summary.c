#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strncmp_rec(const char *s1, const char *s2, size_t n) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));
    
    symbolic c1 = (symbolic)(unsigned long)*(const unsigned char *)s1;
    symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
    cnstr_t eq = _EQ_(c1, c2);
    
    symbolic res;
    if (is_certain(eq)) {
        cnstr_t term = _EQ_(c1, 0);
        if (is_certain(term)) {
            res = (symbolic)0;
        } else if (is_certain(_NOT_(term))) {
            res = strncmp_rec(s1 + 1, s2 + 1, n - 1);
        } else {
            push_pc(); assume(term); symbolic a = (symbolic)0; pop_pc();
            push_pc(); assume(_NOT_(term)); symbolic b = strncmp_rec(s1 + 1, s2 + 1, n - 1); pop_pc();
            res = _ITE_VAR_(term, a, b);
        }
    } else if (is_certain(_NOT_(eq))) {
        res = (symbolic)((unsigned long)c1 - (unsigned long)c2);
    } else {
        push_pc(); assume(eq); symbolic a = (symbolic)0; 
        /* Nested case split for terminator inside the eq branch */
        push_pc(); assume(eq);
        cnstr_t term = _EQ_(c1, 0);
        symbolic inner;
        if (is_certain(term)) { inner = (symbolic)0; }
        else if (is_certain(_NOT_(term))) { inner = strncmp_rec(s1 + 1, s2 + 1, n - 1); }
        else {
            push_pc(); assume(term); symbolic ia = (symbolic)0; pop_pc();
            push_pc(); assume(_NOT_(term)); symbolic ib = strncmp_rec(s1 + 1, s2 + 1, n - 1); pop_pc();
            inner = _ITE_VAR_(term, ia, ib);
        }
        pop_pc();
        a = inner;
        pop_pc();

        push_pc(); assume(_NOT_(eq)); symbolic b = (symbolic)((unsigned long)c1 - (unsigned long)c2); pop_pc();
        res = _ITE_VAR_(eq, a, b);
    }
    
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, res);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    return (int)(unsigned long)strncmp_rec(s1, s2, n);
}
