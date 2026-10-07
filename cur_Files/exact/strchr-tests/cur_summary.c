#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
char *strchr(const char *s, int c) {
    symbolic sym_c = (symbolic)(char)c;
    
    cnstr_t found = _EQ_(*s, sym_c);
    cnstr_t null = _EQ_(*s, '\0');
    
    if (is_certain(found)) {
        return (char *)s;
    }
    
    if (is_certain(_NOT_(found))) {
        if (is_certain(null)) {
            return (char *)0;
        }
        if (is_certain(_NOT_(null))) {
            return strchr(s + 1, c);
        }
        
        push_pc();
        assume(null);
        symbolic r_null = (symbolic)(char *)0;
        pop_pc();
        
        push_pc();
        assume(_NOT_(null));
        symbolic r_rec = (symbolic)strchr(s + 1, c);
        pop_pc();
        
        return (char *)_ITE_VAR_(null, r_null, r_rec);
    }
    
    push_pc();
    assume(found);
    symbolic r_found = (symbolic)(char *)s;
    pop_pc();
    
    push_pc();
    assume(_NOT_(found));
    // Inside this branch, we must handle the terminator logic again
    cnstr_t null_inner = _EQ_(*s, '\0');
    symbolic r_not_found;
    if (is_certain(null_inner)) {
        r_not_found = (symbolic)(char *)0;
    } else if (is_certain(_NOT_(null_inner))) {
        r_not_found = (symbolic)strchr(s + 1, c);
    } else {
        push_pc();
        assume(null_inner);
        symbolic rn = (symbolic)(char *)0;
        pop_pc();
        push_pc();
        assume(_NOT_(null_inner));
        symbolic rr = (symbolic)strchr(s + 1, c);
        pop_pc();
        r_not_found = _ITE_VAR_(null_inner, rn, rr);
    }
    pop_pc();
    
    return (char *)_ITE_VAR_(found, r_found, r_not_found);
}
