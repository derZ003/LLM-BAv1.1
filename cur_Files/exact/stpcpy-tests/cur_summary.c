#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *stpcpy_rec(char *s1, const char *s2) {
    cnstr_t is_null = _EQ_(*s2, '\0');

    if (is_certain(is_null)) {
        *s1 = *s2;
        return s1;
    } else if (is_certain(_NOT_(is_null))) {
        *s1 = *s2;
        return stpcpy_rec(s1 + 1, s2 + 1);
    } else {
        push_pc();
        assume(is_null);
        *s1 = *s2;
        void *res_null = s1;
        pop_pc();

        push_pc();
        assume(_NOT_(is_null));
        *s1 = *s2;
        void *res_not_null = stpcpy_rec(s1 + 1, s2 + 1);
        pop_pc();

        return _ITE_VAR_(is_null, res_null, res_not_null);
    }
}

char *stpcpy(char *dest, const char *src) {
    return (char *)stpcpy_rec(dest, src);
}
