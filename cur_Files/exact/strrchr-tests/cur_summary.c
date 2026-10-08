#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strrchr_rec(const char *s, char c, symbolic last_found) {
    cnstr_t is_null = _EQ_(*s, '\0');
    cnstr_t is_match = _EQ_(*s, c);

    if (is_certain(is_null)) {
        return _ITE_VAR_(is_match, (symbolic)s, last_found);
    }

    if (is_certain(_NOT_(is_null))) {
        symbolic current_found;
        if (is_certain(is_match)) {
            current_found = (symbolic)s;
        } else if (is_certain(_NOT_(is_match))) {
            current_found = last_found;
        } else {
            push_pc(); assume(is_match); symbolic a = (symbolic)s; pop_pc();
            push_pc(); assume(_NOT_(is_match)); symbolic b = last_found; pop_pc();
            current_found = _ITE_VAR_(is_match, a, b);
        }
        return strrchr_rec(s + 1, c, current_found);
    }

    push_pc();
    assume(is_null);
    symbolic r_null = _ITE_VAR_(is_match, (symbolic)s, last_found);
    pop_pc();

    push_pc();
    assume(_NOT_(is_null));
    symbolic current_found;
    if (is_certain(is_match)) {
        current_found = (symbolic)s;
    } else if (is_certain(_NOT_(is_match))) {
        current_found = last_found;
    } else {
        push_pc(); assume(is_match); symbolic a = (symbolic)s; pop_pc();
        push_pc(); assume(_NOT_(is_match)); symbolic b = last_found; pop_pc();
        current_found = _ITE_VAR_(is_match, a, b);
    }
    symbolic r_not_null = strrchr_rec(s + 1, c, current_found);
    pop_pc();

    return _ITE_VAR_(is_null, r_null, r_not_null);
}

char *strrchr(const char *s, int c) {
    return (char *)strrchr_rec(s, (char)c, (symbolic)0);
}
