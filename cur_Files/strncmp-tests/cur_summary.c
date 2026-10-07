#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strncmp_recursive(const char *s1, const char *s2, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return 0;
    } else if (is_certain(_NOT_(n_zero))) {
        symbolic c1 = (symbolic)(unsigned char)*s1;
        symbolic c2 = (symbolic)(unsigned char)*s2;
        cnstr_t eq = _EQ_(c1, c2);
        cnstr_t s1_null = _EQ_(c1, 0);

        if (is_certain(eq)) {
            if (is_certain(s1_null)) {
                return 0;
            } else if (is_certain(_NOT_(s1_null))) {
                return strncmp_recursive(s1 + 1, s2 + 1, n - 1);
            } else {
                push_pc();
                assume(s1_null);
                symbolic r1 = 0;
                pop_pc();
                push_pc();
                assume(_NOT_(s1_null));
                symbolic r2 = strncmp_recursive(s1 + 1, s2 + 1, n - 1);
                pop_pc();
                return _ITE_VAR_(s1_null, r1, r2);
            }
        } else if (is_certain(_NOT_(eq))) {
            return c1 - c2;
        } else {
            push_pc();
            assume(eq);
            symbolic res_eq;
            if (is_certain(s1_null)) {
                res_eq = 0;
            } else if (is_certain(_NOT_(s1_null))) {
                res_eq = strncmp_recursive(s1 + 1, s2 + 1, n - 1);
            } else {
                push_pc();
                assume(s1_null);
                symbolic re1 = 0;
                pop_pc();
                push_pc();
                assume(_NOT_(s1_null));
                symbolic re2 = strncmp_recursive(s1 + 1, s2 + 1, n - 1);
                pop_pc();
                res_eq = _ITE_VAR_(s1_null, re1, re2);
            }
            pop_pc();

            push_pc();
            assume(_NOT_(eq));
            symbolic res_neq = c1 - c2;
            pop_pc();

            return _ITE_VAR_(eq, res_eq, res_neq);
        }
    } else {
        push_pc();
        assume(n_zero);
        symbolic r_zero = 0;
        pop_pc();
        push_pc();
        assume(_NOT_(n_zero));
        symbolic c1 = (symbolic)(unsigned char)*s1;
        symbolic c2 = (symbolic)(unsigned char)*s2;
        cnstr_t eq = _EQ_(c1, c2);
        cnstr_t s1_null = _EQ_(c1, 0);
        symbolic r_nonzero;
        if (is_certain(eq)) {
            if (is_certain(s1_null)) r_nonzero = 0;
            else if (is_certain(_NOT_(s1_null))) r_nonzero = strncmp_recursive(s1 + 1, s2 + 1, n - 1);
            else {
                push_pc(); assume(s1_null); symbolic r1 = 0; pop_pc();
                push_pc(); assume(_NOT_(s1_null)); symbolic r2 = strncmp_recursive(s1 + 1, s2 + 1, n - 1); pop_pc();
                r_nonzero = _ITE_VAR_(s1_null, r1, r2);
            }
        } else if (is_certain(_NOT_(eq))) {
            r_nonzero = c1 - c2;
        } else {
            push_pc();
            assume(eq);
            symbolic res_eq;
            if (is_certain(s1_null)) res_eq = 0;
            else if (is_certain(_NOT_(s1_null))) res_eq = strncmp_recursive(s1 + 1, s2 + 1, n - 1);
            else {
                push_pc(); assume(s1_null); symbolic re1 = 0; pop_pc();
                push_pc(); assume(_NOT_(s1_null)); symbolic re2 = strncmp_recursive(s1 + 1, s2 + 1, n - 1); pop_pc();
                res_eq = _ITE_VAR_(s1_null, re1, re2);
            }
            pop_pc();
            push_pc();
            assume(_NOT_(eq));
            symbolic res_neq = c1 - c2;
            pop_pc();
            r_nonzero = _ITE_VAR_(eq, res_eq, res_neq);
        }
        pop_pc();
        return _ITE_VAR_(n_zero, r_zero, r_nonzero);
    }
}

int strncmp(const char *s1, const char *s2, size_t n) {
    return (int)strncmp_recursive(s1, s2, n);
}
