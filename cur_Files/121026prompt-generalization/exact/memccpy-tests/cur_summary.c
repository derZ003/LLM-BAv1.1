#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic memccpy_rec(char *d, const char *s, size_t n, int c, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return (symbolic)0;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic val = (symbolic)(unsigned long)*(const unsigned char *)s;
    cond_write(d, val, g);
    cnstr_t match = _EQ_(val, (symbolic)(unsigned char)c);

    symbolic r;
    if (is_certain(match)) {
        r = (symbolic)1;
    } else if (is_certain(_NOT_(match))) {
        r = (symbolic)((unsigned long)1 + (unsigned long)memccpy_rec(d + 1, s + 1, n - 1, c, g));
    } else {
        push_pc();
        assume(match);
        symbolic r_match = (symbolic)1;
        pop_pc();
        push_pc();
        assume(_NOT_(match));
        symbolic r_next = (symbolic)((unsigned long)1 + (unsigned long)memccpy_rec(d + 1, s + 1, n - 1, c, _AND_(g, _NOT_(match))));
        pop_pc();
        r = _ITE_VAR_(match, r_match, r_next);
    }
    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, r);
}

void *memccpy(void *s1, const void *s2, int c, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return NULL;
    }
    push_pc();
    assume(_NOT_(n_zero));
    symbolic offset = memccpy_rec((char *)s1, (const char *)s2, n, c, _NOT_(n_zero));
    pop_pc();
    
    symbolic final_offset = _ITE_VAR_(n_zero, (symbolic)0, offset);
    cnstr_t found = _NEQ_(final_offset, 0);
    
    // The library returns r1 (s1 + offset) if n was not exhausted, 
    // or NULL if the loop finished because n reached 0 without a match.
    // Note: n-- is post-decrement, so n is -1 if the loop ran n times without matching.
    // Our offset is n if no match was found.
    cnstr_t not_found = _EQ_(final_offset, (symbolic)n);
    cnstr_t actual_found = _AND_(found, _NOT_(not_found));
    
    // Special case: if n > 0 and the byte at the very last position (n-1) matches,
    // offset == n and it is a match. The library returns s1 + n.
    // In our rec, if match happens at index i (0-based), r = i+1.
    // If match happens at index n-1, r = n.
    // If no match happens, r = n.
    // The library differentiates: if loop ended because n-- became 0, it returns NULL.
    // But if it ended because of the match, it returns r1.
    
    // Let's refine: The loop terminates if (n == 0) OR (match == true).
    // If (match == true), it returns r1.
    // If (n == 0) and no match occurred, it returns NULL.
    
    // In our recursive model:
    // If match occurs at step k (1 <= k <= n), offset = k.
    // If no match occurs after n steps, offset = n.
    
    // The only way to distinguish 'match at step n' from 'no match' is to check 
    // the byte at the last position specifically if offset == n.
    
    push_pc();
    assume(_NOT_(n_zero));
    symbolic last_val = (symbolic)(unsigned long)*(const unsigned char *)((const char *)s2 + (n - 1));
    cnstr_t last_match = _EQ_(last_val, (symbolic)(unsigned char)c);
    
    symbolic res;
    if (is_certain(last_match)) {
        res = _ITE_VAR_(found, (char *)s1 + (unsigned long)final_offset, NULL);
    } else if (is_certain(_NOT_(last_match))) {
        res = _ITE_VAR_(_LT_(final_offset, (symbolic)n), (char *)s1 + (unsigned long)final_offset, NULL);
    } else {
        push_pc();
        assume(last_match);
        symbolic r_last = _ITE_VAR_(found, (char *)s1 + (unsigned long)final_offset, NULL);
        pop_pc();
        push_pc();
        assume(_NOT_(last_match));
        symbolic r_not_last = _ITE_VAR_(_LT_(final_offset, (symbolic)n), (char *)s1 + (unsigned long)final_offset, NULL);
        pop_pc();
        res = _ITE_VAR_(last_match, r_last, r_not_last);
    }
    pop_pc();

    return _ITE_VAR_(n_zero, NULL, res);
}
