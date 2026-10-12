#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strcmp_rec(const char *s1, const char *s2) {
    symbolic c1 = (symbolic)(unsigned long)*(const unsigned char *)s1;
    symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
    cnstr_t end = _EQ_(c1, 0);

    cnstr_t eq = _EQ_(c1, c2);
    if (is_certain(eq)) {
        if (is_certain(end)) {
            return (symbolic)0;
        }
        push_pc(); assume(_NOT_(end));
        symbolic r = strcmp_rec(s1 + 1, s2 + 1);
        pop_pc();
        return _ITE_VAR_(end, (symbolic)0, r);
    } else if (is_certain(_NOT_(eq))) {
        symbolic diff = (symbolic)((unsigned long)c1 - (unsigned long)c2);
        return diff;
    } else {
        push_pc(); assume(eq);
        symbolic r_eq;
        if (is_certain(end)) {
            r_eq = (symbolic)0;
        } else {
            push_pc(); assume(_NOT_(end));
            symbolic r_next = strcmp_rec(s1 + 1, s2 + 1);
            pop_pc();
            r_eq = _ITE_VAR_(end, (symbolic)0, r_next);
        }
        pop_pc();

        push_pc(); assume(_NOT_(eq));
        symbolic r_neq = (symbolic)((unsigned long)c1 - (unsigned long)c2);
        pop_pc();

        return _ITE_VAR_(eq, r_eq, r_neq);
    }
}

int strcmp(const char *s1, const char *s2) {
    return (int)(unsigned long)strcmp_rec(s1, s2);
}
