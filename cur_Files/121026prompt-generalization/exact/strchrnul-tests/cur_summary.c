#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strchrnul_rec(const char *s, symbolic c, cnstr_t guard) {
    symbolic curr_byte = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _OR_(_EQ_(curr_byte, 0), _EQ_(curr_byte, c));

    if (is_certain(end)) {
        return (symbolic)s;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic r = strchrnul_rec((char *)s + 1, c, g);
    pop_pc();

    return _ITE_VAR_(end, (symbolic)s, r);
}

char *strchrnul(const char *s, int c) {
    symbolic sc = (symbolic)(unsigned char)c;
    return (char *)strchrnul_rec(s, sc, TRUE);
}
