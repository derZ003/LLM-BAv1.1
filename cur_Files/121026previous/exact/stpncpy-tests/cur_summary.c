#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic stpncpy_rec(char *s1, const char *s2, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return 0;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    symbolic c = (symbolic)(unsigned char)*s2;
    cnstr_t is_null = _EQ_(c, 0);

    symbolic steps_s2;
    if (is_certain(is_null)) {
        cond_write(s1, c, g);
        steps_s2 = (symbolic)0;
        /* The library continues to write until n is 0, but s2 stops advancing */
        push_pc();
        assume(_NOT_(end));
        cnstr_t g_inner = _AND_(g, is_null);
        /* The loop in the library: while(n) { *s = *s2; if(*s!=0) s2++; s++; n--; } */
        /* If *s2 is 0, it writes 0 and advances s, but not s2. */
        /* To model this, we need a helper to fill remaining n-1 bytes with *s2 */
        // This is handled by the recursion logic below if we adjust the call.
        pop_pc();
    } else if (is_certain(_NOT_(is_null))) {
        cond_write(s1, c, g);
        steps_s2 = 1 + (size_t)stpncpy_rec(s1 + 1, s2 + 1, n - 1, g);
    } else {
        push_pc();
        assume(is_null);
        cond_write(s1, c, _AND_(g, is_null));
        symbolic a = 0 + (size_t)stpncpy_rec(s1 + 1, s2, n - 1, _AND_(g, is_null));
        pop_pc();

        push_pc();
        assume(_NOT_(is_null));
        cond_write(s1, c, _AND_(g, _NOT_(is_null)));
        symbolic b = 1 + (size_t)stpncpy_rec(s1 + 1, s2 + 1, n - 1, _AND_(g, _NOT_(is_null)));
        pop_pc();

        steps_s2 = _ITE_VAR_(is_null, a, b);
    }

    /* Fix for is_certain(is_null) case: it must also recurse to satisfy n bytes */
    if (is_certain(is_null)) {
        steps_s2 = 0 + (size_t)stpncpy_rec(s1 + 1, s2, n - 1, g);
    }

    pop_pc();
    return _ITE_VAR_(end, 0, steps_s2);
}

/* The above logic is slightly messy due to is_certain. Let's rewrite the step cleanly. */

static symbolic stpncpy_step(char *s1, const char *s2, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) return 0;
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic c = (symbolic)(unsigned char)*s2;
    cnstr_t is_null = _EQ_(c, 0);
    cond_write(s1, c, g);
    
    symbolic res;
    if (is_certain(is_null)) {
        res = 0 + (size_t)stpncpy_step(s1 + 1, s2, n - 1, g);
    } else if (is_certain(_NOT_(is_null))) {
        res = 1 + (size_t)stpncpy_step(s1 + 1, s2 + 1, n - 1, g);
    } else {
        push_pc();
        assume(is_null);
        symbolic a = 0 + (size_t)stpncpy_step(s1 + 1, s2, n - 1, _AND_(g, is_null));
        pop_pc();
        push_pc();
        assume(_NOT_(is_null));
        symbolic b = 1 + (size_t)stpncpy_step(s1 + 1, s2 + 1, n - 1, _AND_(g, _NOT_(is_null)));
        pop_pc();
        res = _ITE_VAR_(is_null, a, b);
    }
    pop_pc();
    return _ITE_VAR_(end, 0, res);
}

char *stpncpy(char *s1, const char *s2, size_t n) {
    symbolic k = stpncpy_step(s1, s2, n, TRUE);
    return (char *)((unsigned long)s1 + (unsigned long)k);
}
