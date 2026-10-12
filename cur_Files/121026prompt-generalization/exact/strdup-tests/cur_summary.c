#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void rec_copy(char *d, const char *s, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);
    if (is_certain(end)) {
        cond_write(d, c, guard);
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(d, c, g);
    rec_copy(d + 1, s + 1, g);
    pop_pc();
}

static size_t rec_len(const char *s, size_t n) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);
    if (is_certain(end)) {
        return n;
    }
    push_pc();
    assume(_NOT_(end));
    size_t l = rec_len(s + 1, n + 1);
    pop_pc();
    return _ITE_VAR_(end, (symbolic)n, (symbolic)l);
}

char *strdup(const char *s1) {
    size_t len = rec_len(s1, 0);
    size_t total_size = len + 1;
    char *s = (char *)mem_alloc(total_size);
    if (is_certain(_NEQ_(s, 0))) {
        rec_copy(s, s1, TRUE);
    }
    return s;
}
