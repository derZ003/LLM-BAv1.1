#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static char *strcpy_rec(char *s1, const char *s2) {
    cnstr_t is_null = _EQ_(*s2, '\0');
    
    if (is_certain(is_null)) {
        *s1 = *s2;
        return s1;
    }
    if (is_certain(_NOT_(is_null))) {
        *s1 = *s2;
        return strcpy_rec(s1 + 1, s2 + 1);
    }

    push_pc();
    assume(is_null);
    *s1 = *s2;
    char *res_null = s1;
    pop_pc();

    push_pc();
    assume(_NOT_(is_null));
    *s1 = *s2;
    char *res_not_null = strcpy_rec(s1 + 1, s2 + 1);
    pop_pc();

    // Must ensure the write happens regardless of the path chosen
    // because it happens before the loop condition check in the libc code.
    cond_write(s1, *s2, TRUE);
    return _ITE_VAR_(is_null, res_null, res_not_null);
}

char *strcpy(char *dest, const char *src) {
    return strcpy_rec(dest, src);
}
