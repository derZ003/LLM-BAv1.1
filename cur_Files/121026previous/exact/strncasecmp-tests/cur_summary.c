#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic sym_tolower(symbolic c) {
    cnstr_t is_upper = _AND_(_GE_(c, (symbolic)'A'), _LE_(c, (symbolic)'Z'));
    symbolic lower = (symbolic)((unsigned long)c + 32);
    if (is_certain(is_upper)) return lower;
    if (is_certain(_NOT_(is_upper))) return c;
    push_pc(); assume(is_upper); symbolic a = lower; pop_pc();
    push_pc(); assume(_NOT_(is_upper)); symbolic b = c; pop_pc();
    return _ITE_VAR_(is_upper, a, b);
}

static symbolic rec_strncasecmp(const char *s1, const char *s2, size_t n) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) { return (symbolic)0; }
    push_pc(); assume(_NOT_(end));
    
    symbolic c1 = (symbolic)(unsigned long)*(const unsigned char *)s1;
    symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
    symbolic lc1 = sym_tolower(c1);
    symbolic lc2 = sym_tolower(c2);
    
    cnstr_t eq = _EQ_(lc1, lc2);
    cnstr_t s1_null = _EQ_(c1, 0);
    
    symbolic r;
    if (is_certain(eq)) {
        if (is_certain(s1_null)) {
            r = (symbolic)0;
        } else if (is_certain(_NOT_(s1_null))) {
            r = rec_strncasecmp(s1 + 1, s2 + 1, n - 1);
        } else {
            push_pc(); assume(s1_null); symbolic a = (symbolic)0; pop_pc();
            push_pc(); assume(_NOT_(s1_null)); symbolic b = rec_strncasecmp(s1 + 1, s2 + 1, n - 1); pop_pc();
            r = _ITE_VAR_(s1_null, a, b);
        }
    } else if (is_certain(_NOT_(eq))) {
        r = (symbolic)((unsigned long)lc1 - (unsigned long)lc2);
    } else {
        push_pc(); assume(eq); 
        symbolic eq_res;
        if (is_certain(s1_null)) { eq_res = (symbolic)0; }
        else if (is_certain(_NOT_(s1_null))) { eq_res = rec_strncasecmp(s1 + 1, s2 + 1, n - 1); }
        else {
            push_pc(); assume(s1_null); symbolic a = (symbolic)0; pop_pc();
            push_pc(); assume(_NOT_(s1_null)); symbolic b = rec_strncasecmp(s1 + 1, s2 + 1, n - 1); pop_pc();
            eq_res = _ITE_VAR_(s1_null, a, b);
        }
        pop_pc();
        
        push_pc(); assume(_NOT_(eq));
        symbolic neq_res = (symbolic)((unsigned long)lc1 - (unsigned long)lc2);
        pop_pc();
        
        r = _ITE_VAR_(eq, eq_res, neq_res);
    }
    
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    return (int)(unsigned long)rec_strncasecmp(s1, s2, n);
}
