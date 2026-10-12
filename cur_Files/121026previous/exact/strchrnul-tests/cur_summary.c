#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strchrnul_rec(const char *s, int c) {
    symbolic cur = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _OR_(_EQ_(cur, 0), _EQ_(cur, (symbolic)(unsigned char)c));
    if (is_certain(end)) {
        return (symbolic)s;
    }
    push_pc();
    assume(_NOT_(end));
    symbolic r = strchrnul_rec(s + 1, c);
    pop_pc();
    return _ITE_VAR_(end, (symbolic)s, r);
}

char *strchrnul(const char *s, int c) {
    return (char *)strchrnul_rec(s, c);
}
