#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strcpy_rec(char *d, const char *s, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);

    cond_write(d, c, guard);

    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    strcpy_rec(d + 1, s + 1, g);
    pop_pc();
}

char *strcpy(char *dest, const char *src) {
    strcpy_rec(dest, src, TRUE);
    return dest;
}
