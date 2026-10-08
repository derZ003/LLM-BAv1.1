#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic _strchrnul_recursive(const char *s, int c) {
    symbolic current_char = *s;
    cnstr_t is_null = _EQ_(current_char, '\0');
    cnstr_t is_match = _EQ_(current_char, (char)c);

    if (is_certain(is_null)) {
        return (symbolic)s;
    }
    if (is_certain(_NOT_(is_null)) && is_certain(is_match)) {
        return (symbolic)s;
    }
    if (is_certain(_NOT_(is_null)) && is_certain(_NOT_(is_match))) {
        return _strchrnul_recursive(s + 1, c);
    }

    // Case split for the combined termination condition: (*s == '\0' || *s == (char)c)
    cnstr_t stop = _OR_(is_null, is_match);
    
    push_pc();
    assume(stop);
    symbolic res_stop = (symbolic)s;
    pop_pc();

    push_pc();
    assume(_NOT_(stop));
    symbolic res_cont = _strchrnul_recursive(s + 1, c);
    pop_pc();

    return _ITE_VAR_(stop, res_stop, res_cont);
}

char *strchrnul(const char *s, int c) {
    return (char *)_strchrnul_recursive(s, c);
}
