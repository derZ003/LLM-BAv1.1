#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strcmp_recursive(const char *s1, const char *s2) {
    symbolic c1 = (symbolic)(unsigned char)*s1;
    symbolic c2 = (symbolic)(unsigned char)*s2;

    cnstr_t eq = _EQ_(c1, c2);
    cnstr_t s1_zero = _EQ_(c1, (symbolic)0);

    if (is_certain(eq)) {
        if (is_certain(s1_zero)) {
            return (symbolic)0;
        } else if (is_certain(_NOT_(s1_zero))) {
            return strcmp_recursive(s1 + 1, s2 + 1);
        } else {
            push_pc();
            assume(s1_zero);
            symbolic r1 = (symbolic)0;
            pop_pc();
            push_pc();
            assume(_NOT_(s1_zero));
            symbolic r2 = strcmp_recursive(s1 + 1, s2 + 1);
            pop_pc();
            return _ITE_VAR_(s1_zero, r1, r2);
        }
    } else if (is_certain(_NOT_(eq))) {
        symbolic diff = (symbolic)((int)(unsigned char)*s1) - (int)(unsigned char)*s2;
        return diff;
    } else {
        push_pc();
        assume(eq);
        symbolic r_eq;
        if (is_certain(s1_zero)) {
            r_eq = (symbolic)0;
        } else if (is_certain(_NOT_(s1_zero))) {
            r_eq = strcmp_recursive(s1 + 1, s2 + 1);
        } else {
            push_pc();
            assume(s1_zero);
            symbolic re1 = (symbolic)0;
            pop_pc();
            push_pc();
            assume(_NOT_(s1_zero));
            symbolic re2 = strcmp_recursive(s1 + 1, s2 + 1);
            pop_pc();
            r_eq = _ITE_VAR_(s1_zero, re1, re2);
        }
        pop_pc();

        push_pc();
        assume(_NOT_(eq));
        symbolic diff = (symbolic)((int)(unsigned char)*s1) - (int)(unsigned char)*s2;
        symbolic r_neq = diff;
        pop_pc();

        return _ITE_VAR_(eq, r_eq, r_neq);
    }
}

int strcmp(const char *s1, const char *s2) {
    return (int)strcmp_recursive(s1, s2);
}
