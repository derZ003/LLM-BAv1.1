static symbolic sym_tolower(symbolic c) {
    cnstr_t is_upper = _AND_(_GE_(c, 'A'), _LE_(c, 'Z'));
    return _ITE_VAR_(is_upper, c + ('a' - 'A'), c);
}

static int strcasecmp_recursive(const char *_l, const char *_r, size_t i) {
    allocd((void *)_l, i + 1);
    allocd((void *)_r, i + 1);
    
    symbolic l_val = (symbolic)((unsigned char)_l[i]);
    symbolic r_val = (symbolic)((unsigned char)_r[i]);
    
    symbolic tl = sym_tolower(l_val);
    symbolic tr = sym_tolower(r_val);
    
    cnstr_t l_null = _EQ_(l_val, 0);
    cnstr_t r_null = _EQ_(r_val, 0);
    cnstr_t eq_case = _EQ_(tl, tr);
    
    cnstr_t loop_cond = _AND_(_NOT_(l_null), _AND_(_NOT_(r_null), eq_case));
    
    if (is_certain(loop_cond)) {
        return strcasecmp_recursive(_l, _r, i + 1);
    } else if (is_certain(_NOT_(loop_cond))) {
        return (int)(tl - tr);
    } else {
        push_pc();
        assume(loop_cond);
        int res_continue = strcasecmp_recursive(_l, _r, i + 1);
        pop_pc();
        
        push_pc();
        assume(_NOT_(loop_cond));
        int res_break = (int)(tl - tr);
        pop_pc();
        
        return (int)_ITE_VAR_(loop_cond, (symbolic)res_continue, (symbolic)res_break);
    }
}

int strcasecmp(const char *_l, const char *_r) {
    return strcasecmp_recursive(_l, _r, 0);
}
