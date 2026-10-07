#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlen_rec(const char *s, size_t offset) {
    symbolic curr_char = *(s + offset);
    cnstr_t is_null = _EQ_(curr_char, (symbolic)0);

    if (is_certain(is_null)) {
        return (symbolic)offset;
    }

    if (is_certain(_NOT_(is_null))) {
        return strlen_rec(s, offset + 1);
    }

    push_pc();
    assume(is_null);
    symbolic res_null = (symbolic)offset;
    pop_pc();

    push_pc();
    assume(_NOT_(is_null));
    symbolic res_rec = strlen_rec(s, offset + 1);
    pop_pc();

    return _ITE_VAR_(is_null, res_null, res_rec);
}

size_t strlen(const char *s) {
    return (size_t)strlen_rec(s, 0);
}
