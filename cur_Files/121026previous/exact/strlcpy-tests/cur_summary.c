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
    if (is_certain(_EQ_(n, 0))) {
        res = strlcpy_rec(dst, src + 1, 0, g);
    } else if (is_certain(_NEQ_(n, 0))) {
        res = strlcpy_rec(dst + 1, src + 1, n - 1, g);
    } else {
        push_pc();
        assume(_NEQ_(n, 0));
        symbolic r_pos = strlcpy_rec(dst + 1, src + 1, n - 1, _AND_(g, _NEQ_(n, 0)));
        pop_pc();
        
        push_pc();
        assume(_EQ_(n, 0));
        symbolic r_zero = strlcpy_rec(dst, src + 1, 0, _AND_(g, _EQ_(n, 0)));
        pop_pc();
        
        res = _ITE_VAR_(_NEQ_(n, 0), r_pos, r_zero);
    }
    
    symbolic step = (symbolic)1;
    symbolic total = (symbolic)((unsigned long)res + (unsigned long)step);
    pop_pc();
    
    return _ITE_VAR_(end, (symbolic)0, total);
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    
    if (is_certain(n_zero)) {
        return (size_t)(unsigned long)strlcpy_rec((char *)0, src, 0, FALSE);
    } else if (is_certain(_NOT_(n_zero))) {
        return (size_t)(unsigned long)strlcpy_rec(dst, src, n - 1, TRUE);
    } else {
        push_pc();
        assume(n_zero);
        symbolic r_z = strlcpy_rec((char *)0, src, 0, FALSE);
        pop_pc();
        
        push_pc();
        assume(_NOT_(n_zero));
        symbolic r_nz = strlcpy_rec(dst, src, n - 1, _NOT_(n_zero));
        pop_pc();
        
        return (size_t)(unsigned long)_ITE_VAR_(n_zero, r_z, r_nz);
    }
}
