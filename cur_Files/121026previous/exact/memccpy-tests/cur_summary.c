#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memccpy_rec(char *d, const char *s, size_t n, int c, cnstr_t guard) {
    cnstr_t end_len = _EQ_(n, 0);
    if (is_certain(end_len)) {
        return (symbolic)0; // sentinel for "not found" and reached end of length
    }

    push_pc();
    assume(_NOT_(end_len));
    cnstr_t g = _AND_(guard, _NOT_(end_len));

    symbolic val = (symbolic)(unsigned long)*(const unsigned char *)s;
    cond_write(d, val, g);

    cnstr_t match = _EQ_(val, (symbolic)(unsigned char)c);
    if (is_certain(match)) {
        // Found match: return the pointer to the byte after the match
        symbolic res = (symbolic)(unsigned long)d + 1;
        pop_pc();
        return _ITE_VAR_(end_len, (symbolic)0, res);
    } else if (is_certain(_NOT_(match))) {
        // No match: recurse
        symbolic res = memccpy_rec(d + 1, s + 1, n - 1, c, g);
        // If recursion returns 0, it means it reached n=0 without finding c
        // We must distinguish between "not found" (NULL) and "found at last byte"
        // The library returns r1 (which is d+1) if match occurs, else NULL if n wraps.
        pop_pc();
        return _ITE_VAR_(end_len, (symbolic)0, res);
    } else {
        // Match is undecided
        push_pc();
        assume(match);
        symbolic res_m = (symbolic)(unsigned long)d + 1;
        pop_pc();

        push_pc();
        assume(_NOT_(match));
        symbolic res_nm = memccpy_rec(d + 1, s + 1, n - 1, c, _AND_(g, _NOT_(match)));
        pop_pc();

        symbolic res = _ITE_VAR_(match, res_m, res_nm);
        pop_pc();
        return _ITE_VAR_(end_len, (symbolic)0, res);
    }
}

void *memccpy(void *s1, const void *s2, int c, size_t n) {
    if (is_certain(_EQ_(n, 0))) {
        return s1;
    }

    symbolic res = memccpy_rec((char *)s1, (const char *)s2, n, c, TRUE);

    // The library returns NULL if the loop finished because n became -1 (exhausted n)
    // and no match was found. Otherwise it returns r1.
    // In our recursion, (symbolic)0 represents the "not found" state.
    cnstr_t found = _NEQ_(res, (symbolic)0);
    if (is_certain(found)) {
        return (void *)res;
    } else if (is_certain(_NOT_(found))) {
        return NULL;
    } else {
        push_pc();
        assume(found);
        void *a = (void *)res;
        pop_pc();

        push_pc();
        assume(_NOT_(found));
        void *b = NULL;
        pop_pc();

        return (void *)_ITE_VAR_(found, a, b);
    }
}
