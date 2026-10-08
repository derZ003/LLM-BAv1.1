#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strncpy_rec(char *s1, const char *s2, size_t n, cnstr_t guard) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return;
    }

    symbolic val = *s2;
    cnstr_t is_null = _EQ_(val, 0);
    cnstr_t g_write = _AND_(guard, _NOT_(n_zero));
    cond_write(s1, val, g_write);

    cnstr_t s2_inc_guard = _AND_(g_write, _NOT_(is_null));
    cnstr_t s2_stay_guard = _AND_(g_write, is_null);

    // We must decide how to proceed with s2 and n for the next step
    // If *s2 was null, we continue writing nulls until n == 0
    symbolic next_s2 = _ITE_VAR_(is_null, (symbolic)s2, (symbolic)(s2 + 1));
    
    push_pc();
    assume(_NOT_(n_zero));
    strncpy_rec(s1 + 1, (const char *)next_s2, n - 1, g_write);
    pop_pc();
}

char *strncpy(char *dest, const char *src, size_t n) {
    strncpy_rec(dest, src, n, TRUE);
    return dest;
}
