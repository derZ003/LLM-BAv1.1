int tolower(int c) {
    cnstr_t is_upper = _AND_(_GE_(c, 'A'), _LE_(c, 'Z'));
    
    if (is_certain(is_upper)) {
        return c | 32;
    } else if (is_certain(_NOT_(is_upper))) {
        return c;
    } else {
        push_pc();
        assume(is_upper);
        int upper_res = c | 32;
        pop_pc();
        
        push_pc();
        assume(_NOT_(is_upper));
        int lower_res = c;
        pop_pc();
        
        return _ITE_VAR_(is_upper, upper_res, lower_res);
    }
}
