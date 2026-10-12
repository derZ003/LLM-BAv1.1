#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlen_rec(const char *s, size_t len) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);

    if (is_certain(end)) {
        return (symbolic)len;
    }

    push_pc();
    assume(_NOT_(end));
    symbolic r = strlen_rec(s + 1, len + 1);
    pop_pc();

    return _ITE_VAR_(end, (symbolic)len, r);
}

size_t strlen(const char *s) {
    return (size_t)strlen_rec(s, 0);
}
