#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strnlen_rec(const char *s, size_t n) {
    cnstr_t end_n = _EQ_(n, 0);
    if (is_certain(end_n)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end_n));
    
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end_c = _EQ_(c, 0);
    
    symbolic r;
    if (is_certain(end_c)) {
        r = (symbolic)0;
    } else if (is_certain(_NOT_(end_c))) {
        symbolic res = strnlen_rec(s + 1, n - 1);
        r = (symbolic)((unsigned long)res + 1);
    } else {
        push_pc();
        assume(end_c);
        symbolic a = (symbolic)0;
        pop_pc();
        
        push_pc();
        assume(_NOT_(end_c));
        symbolic res = strnlen_rec(s + 1, n - 1);
        symbolic b = (symbolic)((unsigned long)res + 1);
        pop_pc();
        
        r = _ITE_VAR_(end_c, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end_n, (symbolic)0, r);
}

size_t strnlen(const char *s, size_t max) {
    return (size_t)(unsigned long)strnlen_rec(s, max);
}
