#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void memcpy_rec(char *d, const char *s, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cond_write(d, c, g);
    memcpy_rec(d + 1, s + 1, n - 1, g);
    pop_pc();
}

void *memcpy(void *dest, const void *src, size_t n) {
    memcpy_rec((char *)dest, (const char *)src, n, TRUE);
    return dest;
}
