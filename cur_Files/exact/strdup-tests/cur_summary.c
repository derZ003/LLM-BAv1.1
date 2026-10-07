static size_t sym_strlen(const char *s) {
    if (is_certain(_EQ_(*s, 0))) {
        return 0;
    }
    return 1 + sym_strlen(s + 1);
}

char *strdup(const char *s) {
    size_t len = sym_strlen(s);
    char *d = (char *)mem_alloc(len + 1);
    
    if (d == NULL) {
        return NULL;
    }

    for (size_t i = 0; i <= len; i++) {
        cond_write(d + i, *(s + i), _NOT_(_EQ_(0, 0))); // Always write current char
    }

    return d;
}
