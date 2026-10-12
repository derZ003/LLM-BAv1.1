#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strcmp_rec(const char *s1, const char *s2) {
    symbolic c1 = (symbolic)(unsigned char)*s1;
    symbolic c2 = (symbolic)(unsigned char)*s2;
    cnstr_t end = _EQ_(c1, 0);
    
    if (is_certain(end)) {
        return (symbolic)((int)c1 - (int)c2);
    }

    push_pc();
    assume(_NOT_(end));
    
    cnstr_t eq = _EQ_(c1, c2);
    symbolic r;
    if (is_certain(eq)) {
        r = strcmp_rec(s1 + 1, s2 + 1);
    } else if (is_certain(_NOT_(eq))) {
        r = (symbolic)((int)c1 - (int)c2);
    } else {
        push_pc();
        assume(eq);
        symbolic r_eq = strcmp_rec(s1 + 1, s2 + 1);
        pop_pc();
        
        push_pc();
        assume(_NOT_(eq));
        symbolic r_neq = (symbolic)((int)c1 - (int)c2);
        pop_pc();
        
        r = _ITE_VAR_(eq, r_eq, r_neq);
    }
    pop_pc();

    return _ITE_VAR_(end, (symbolic)((int)c1 - (int)c2), r);
}

int strcmp(const char *s1, const char *s2) {
    return (int)strcmp_rec(s1, s2);
}
