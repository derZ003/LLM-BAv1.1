#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strrchr_rec(const char *s, char c, symbolic last_found, cnstr_t guard) {
    symbolic curr_byte = (symbolic)(unsigned char)*s;
    cnstr_t end = _EQ_(curr_byte, 0);

    if (is_certain(end)) {
        // The library loop: do { if (*s == c) p = s; } while (*s++);
        // If curr_byte is 0, it is still checked against c before the loop terminates.
        cnstr_t match_end = _EQ_(curr_byte, (symbolic)c);
        if (is_certain(match_end)) {
            return (symbolic)s;
        } else if (is_certain(_NOT_(match_end))) {
            return last_found;
        } else {
            push_pc();
            assume(match_end);
            symbolic a = (symbolic)s;
            pop_pc();
            push_pc();
            assume(_NOT_(match_end));
            symbolic b = last_found;
            pop_pc();
            return _ITE_VAR_(match_end, a, b);
        }
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    cnstr_t match = _EQ_(curr_byte, (symbolic)c);
    symbolic next_found;
    if (is_certain(match)) {
        next_found = (symbolic)s;
    } else if (is_certain(_NOT_(match))) {
        next_found = last_found;
    } else {
        push_pc();
        assume(match);
        symbolic a = (symbolic)s;
        pop_pc();
        push_pc();
        assume(_NOT_(match));
        symbolic b = last_found;
        pop_pc();
        next_found = _ITE_VAR_(match, a, b);
    }

    symbolic r = strrchr_rec(s + 1, c, next_found, g);
    pop_pc();

    return _ITE_VAR_(end, (symbolic)0, r); // The 'end' case is handled by the is_certain block above; this is just for the skeleton.
}

// Correcting the skeleton logic for strrchr: the loop is a do-while.
static symbolic strrchr_rec_fixed(const char *s, char c, symbolic last_found, cnstr_t guard) {
    symbolic curr_byte = (symbolic)(unsigned char)*s;
    cnstr_t end = _EQ_(curr_byte, 0);

    // Step logic: check current byte, then decide whether to continue.
    cnstr_t match = _EQ_(curr_byte, (symbolic)c);
    symbolic current_found;
    if (is_certain(match)) {
        current_found = (symbolic)s;
    } else if (is_certain(_NOT_(match))) {
        current_found = last_found;
    } else {
        push_pc(); assume(match); symbolic a = (symbolic)s; pop_pc();
        push_pc(); assume(_NOT_(match)); symbolic b = last_found; pop_pc();
        current_found = _ITE_VAR_(match, a, b);
    }

    if (is_certain(end)) {
        return current_found;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic r = strrchr_rec_fixed(s + 1, c, current_found, g);
    pop_pc();

    return _ITE_VAR_(end, current_found, r);
}

char *strrchr(const char *s, int c) {
    return (char *)strrchr_rec_fixed(s, (char)c, (symbolic)0, TRUE);
}
