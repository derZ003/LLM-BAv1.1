#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strchr_rec(const char *s, symbolic c) {
    symbolic curr_byte = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t found = _EQ_(curr_byte, c);
    cnstr_t end = _EQ_(curr_byte, 0);

    if (is_certain(found)) {
        return (symbolic)s;
    }

    /* The library loop is: do { if (*s == c) return s; } while (*s++);
       If c == 0, the null terminator is a match and is returned.
       If c != 0, the loop terminates after the null terminator is encountered. */
    if (is_certain(end)) {
        return _ITE_VAR_(found, (symbolic)s, (symbolic)0);
    }

    push_pc();
    assume(_NOT_(end));

    symbolic r;
    if (is_certain(found)) {
        r = (symbolic)s;
    } else if (is_certain(_NOT_(found))) {
        r = strchr_rec((const char *)s + 1, c);
    } else {
        push_pc();
        assume(found);
        symbolic a = (symbolic)s;
        pop_pc();
        push_pc();
        assume(_NOT_(found));
        symbolic b = strchr_rec((const char *)s + 1, c);
        pop_pc();
        r = _ITE_VAR_(found, a, b);
    }
    pop_pc();

    return _ITE_VAR_(end, _ITE_VAR_(found, (symbolic)s, (symbolic)0), r);
}

char *strchr(const char *s, int c) {
    return (char *)strchr_rec(s, (symbolic)(unsigned char)c);
}
