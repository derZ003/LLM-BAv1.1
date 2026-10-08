#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strdup_rec_len(const char *s, size_t n) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);
    if (is_certain(end)) {
        return (symbolic)n;
    }
    push_pc();
    assume(_NOT_(end));
    symbolic r = strdup_rec_len(s + 1, n + 1);
    pop_pc();
    return _ITE_VAR_(end, (symbolic)n, r);
}

static void strdup_rec_copy(char *dest, const char *src, size_t n, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)src;
    cnstr_t end = _EQ_(c, 0);
    if (is_certain(end)) {
        cond_write(dest, 0, guard);
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(dest, c, g);
    strdup_rec_copy(dest + 1, src + 1, n + 1, g);
    pop_pc();
}

char *strdup(const char *s1) {
    symbolic len = strdup_rec_len(s1, 0);
    size_t size = (size_t)len + 1;
    char *s = (char *)mem_alloc(size);
    if (s != NULL) {
        strdup_rec_copy(s, s1, 0, TRUE);
    }
    return s;
}
