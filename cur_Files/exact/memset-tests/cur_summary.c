#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void *memset_recursive(void *s, int c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return s;
    }
    if (is_certain(_NOT_(n_zero))) {
        allocd(s, 1);
        cond_write(s, (symbolic)(unsigned char)c, TRUE);
        return memset_recursive((char *)s + 1, c, n - 1);
    }
    push_pc();
    assume(n_zero);
    void *res_zero = s;
    pop_pc();
    push_pc();
    assume(_NOT_(n_zero));
    allocd(s, 1);
    cond_write(s, (symbolic)(unsigned char)c, TRUE);
    void *res_step = memset_recursive((char *)s + 1, c, n - 1);
    pop_pc();
    
    /* 
     * The return value of memset is always the original pointer s.
     * The recursive call returns s + n, but the API requires s.
     * The original summary was using the return value of the recursive step,
     * which is why it failed. We must return s in both branches.
     */
    return _ITE_VAR_(n_zero, (symbolic)res_zero, (symbolic)res_zero);
}

void *memset(void *s, int c, size_t n) {
    return memset_recursive(s, c, n);
}
