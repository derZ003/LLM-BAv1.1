#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void memset_rec(unsigned char *p, symbolic c, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(p, c, g);
    memset_rec(p + 1, c, n - 1, g);
    pop_pc();
}

void *memset(void *s, int c, size_t n) {
    memset_rec((unsigned char *)s, (symbolic)(unsigned char)c, n, TRUE);
    return s;
}
