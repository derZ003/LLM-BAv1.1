#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic sym_tolower(symbolic c) {
    cnstr_t is_upper = _AND_(_GE_(c, (symbolic)'A'), _LE_(c, (symbolic)'Z'));
    symbolic lower = (symbolic)((unsigned long)c + 32);
    if (is_certain(is_upper)) return lower;
    if (is_certain(_NOT_(is_upper))) return c;
    push_pc(); assume(is_upper); symbolic a = lower; pop_pc();
    push_pc(); assume(_NOT_(is_upper)); symbolic b = c; pop_pc();
    return _ITE_VAR_(is_upper, a, b);
}

static symbolic strcasecmp_rec(const char *s1, const char *s2) {
    symbolic c1 = (symbolic)(unsigned long)*(const unsigned char *)s1;
    cnstr_t end = _EQ_(c1, 0);
    if (is_certain(end)) {
        symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
        symbolic lc2 = sym_tolower(c2);
        return (symbolic)((unsigned long)0 - (unsigned long)lc2);
    }
    push_pc(); assume(_NOT_(end));
    symbolic c2 = (symbolic)(unsigned long)*(const unsigned char *)s2;
    symbolic lc1 = sym_tolower(c1);
    symbolic lc2 = sym_tolower(c2);
    cnstr_t eq = _EQ_(lc1, lc2);
    symbolic r;
    if (is_certain(eq)) {
        r = strcasecmp_rec(s1 + 1, s2 + 1);
    } else if (is_certain(_NOT_(eq))) {
        r = (symbolic)((unsigned long)lc1 - (unsigned long)lc2);
    } else {
        push_pc(); assume(eq); symbolic a = strcasecmp_rec(s1 + 1, s2 + 1); pop_pc();
        push_pc(); assume(_NOT_(eq)); symbolic b = (symbolic)((unsigned long)lc1 - (unsigned long)lc2); pop_pc();
        r = _ITE_VAR_(eq, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)((unsigned long)0 - (unsigned long)sym_tolower((symbolic)(unsigned long)*(const unsigned char *)s2)), r);
}

int strcasecmp(const char *s1, const char *s2) {
    return (int)(unsigned long)strcasecmp_rec(s1, s2);
}
