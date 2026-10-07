#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strcpy_recursive(char *s1, const char *s2) {
    cnstr_t is_null = _EQ_(*s2, 0);
    if (is_certain(is_null)) {
        *s1 = *s2;
    } else if (is_certain(_NOT_(is_null))) {
        *s1 = *s2;
        strcpy_recursive(s1 + 1, s2 + 1);
    } else {
        push_pc();
        assume(is_null);
        *s1 = *s2;
        pop_pc();

        push_pc();
        assume(_NOT_(is_null));
        *s1 = *s2;
        strcpy_recursive(s1 + 1, s2 + 1);
        pop_pc();
    }
}

char *strcpy(char *dest, const char *src) {
    strcpy_recursive(dest, src);
    return dest;
}
