#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strnlen_rec(const char *s, size_t max) {
    cnstr_t max_zero = _EQ_(max, 0);
    if (is_certain(max_zero)) {
        return 0;
    }

    allocd(s, 1);
    cnstr_t s_null = _EQ_(*s, '\0');
    if (is_certain(s_null)) {
        return 0;
    }

    if (is_certain(_NOT_(max_zero)) && is_certain(_NOT_(s_null))) {
        return 1 + strnlen_rec(s + 1, max - 1);
    }

    push_pc();
    assume(max_zero);
    symbolic r_zero = 0;
    pop_pc();

    push_pc();
    assume(_NOT_(max_zero));
    push_pc();
    assume(s_null);
    symbolic r_null = 0;
    pop_pc();

    push_pc();
    assume(_NOT_(s_null));
    symbolic r_rec = 1 + strnlen_rec(s + 1, max - 1);
    pop_pc();
    symbolic r_not_zero = _ITE_VAR_(s_null, r_null, r_rec);
    pop_pc();

    return _ITE_VAR_(max_zero, r_zero, r_not_zero);
}

size_t strnlen(const char *s, size_t max) {
    return (size_t)strnlen_rec(s, max);
}
