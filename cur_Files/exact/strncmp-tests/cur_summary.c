#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strncmp_rec(const char *s1, const char *s2, size_t n) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return 0;
    }
    push_pc();
    assume(_NOT_(end));
    
    symbolic c1 = (symbolic)(unsigned long)*(const unsigned char *)s1;
    symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
    cnstr_t eq = _EQ_(c1, c2);
    cnstr_t zero = _EQ_(c1, 0);
    
    symbolic r;
    if (is_certain(eq)) {
        if (is_certain(zero)) {
            r = 0;
        } else if (is_certain(_NOT_(zero))) {
            r = strncmp_rec(s1 + 1, s2 + 1, n - 1);
        } else {
            push_pc(); assume(zero); symbolic r_z = 0; pop_pc();
            push_pc(); assume(_NOT_(zero)); symbolic r_nz = strncmp_rec(s1 + 1, s2 + 1, n - 1); pop_pc();
            r = _ITE_VAR_(zero, r_z, r_nz);
        }
    } else if (is_certain(_NOT_(eq))) {
        r = (symbolic)((int)c1 - (int)c2);
    } else {
        push_pc(); assume(eq); 
        symbolic r_eq;
        if (is_certain(zero)) {
            r_eq = 0;
        } else if (is_certain(_NOT_(zero))) {
            r_eq = strncmp_rec(s1 + 1, s2 + 1, n - 1);
        } else {
            push_pc(); assume(zero); symbolic r_z = 0; pop_pc();
            push_pc(); assume(_NOT_(zero)); symbolic r_nz = strncmp_rec(s1 + 1, s2 + 1, n - 1); pop_pc();
            r_eq = _ITE_VAR_(zero, r_z, r_nz);
        }
        pop_pc();
        
        push_pc(); assume(_NOT_(eq));
        symbolic r_neq = (symbolic)((int)c1 - (int)c2);
        pop_pc();
        
        r = _ITE_VAR_(eq, r_eq, r_neq);
    }
    
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    return (int)strncmp_rec(s1, s2, n);
}
