#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic stpcpy_rec(char *d, const char *s, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);

    if (is_certain(end)) {
        cond_write(d, 0, guard);
        return (symbolic)d;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(d, c, g);
    symbolic r = stpcpy_rec(d + 1, s + 1, g);
    pop_pc();

    cond_write(d, 0, _AND_(guard, end));
    return _ITE_VAR_(end, (symbolic)d, r);
}

char *stpcpy(char *dest, const char *src) {
    return (char *)stpcpy_rec(dest, src, TRUE);
}
