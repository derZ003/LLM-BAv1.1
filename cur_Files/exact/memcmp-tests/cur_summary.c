#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memcmp_rec(const unsigned char *s1, const unsigned char *s2, size_t n) {
    cnstr_t is_zero = _EQ_(n, (symbolic)0);
    if (is_certain(is_zero)) {
        return (symbolic)0;
    } else if (is_certain(_NOT_(is_zero))) {
        symbolic v1 = (symbolic)*s1;
        symbolic v2 = (symbolic)*s2;
        cnstr_t eq = _EQ_(v1, v2);

        if (is_certain(eq)) {
            return memcmp_rec(s1 + 1, s2 + 1, n - 1);
        } else if (is_certain(_NOT_(eq))) {
            cnstr_t lt = _ULT_(v1, v2);
            if (is_certain(lt)) {
                return (symbolic)((int)v1 - (int)v2);
            } else if (is_certain(_NOT_(lt))) {
                return (symbolic)((int)v1 - (int)v2);
            } else {
                push_pc();
                assume(lt);
                symbolic r_lt = (symbolic)((int)v1 - (int)v2);
                pop_pc();
                push_pc();
                assume(_NOT_(lt));
                symbolic r_gt = (symbolic)((int)v1 - (int)v2);
                pop_pc();
                return _ITE_VAR_(lt, r_lt, r_gt);
            }
        } else {
            push_pc();
            assume(eq);
            symbolic r_eq = memcmp_rec(s1 + 1, s2 + 1, n - 1);
            pop_pc();
            push_pc();
            assume(_NOT_(eq));
            symbolic r_neq = (symbolic)((int)v1 - (int)v2);
            pop_pc();
            return _ITE_VAR_(eq, r_eq, r_neq);
        }
    } else {
        push_pc();
        assume(is_zero);
        symbolic r_zero = (symbolic)0;
        pop_pc();
        push_pc();
        assume(_NOT_(is_zero));
        symbolic r_nonzero = memcmp_rec(s1, s2, n);
        pop_pc();
        return _ITE_VAR_(is_zero, r_zero, r_nonzero);
    }
}

int memcmp(const void *s1, const void *s2, size_t n) {
    allocd(s1, n);
    allocd(s2, n);
    return (int)memcmp_rec((const unsigned char *)s1, (const unsigned char *)s2, n);
}
