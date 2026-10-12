#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strrchr_rec(const char *s, symbolic c, symbolic last_match, cnstr_t guard) {
    symbolic curr_byte = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(curr_byte, 0);

    if (is_certain(end)) {
        // The library's loop is do-while. Even if the first byte is '\0',
        // it checks it against 'c' before terminating.
        cnstr_t match = _EQ_(curr_byte, c);
        if (is_certain(match)) {
            return (symbolic)s;
        } else if (is_certain(_NOT_(match))) {
            return last_match;
        } else {
            push_pc();
            assume(match);
            symbolic m_true = (symbolic)s;
            pop_pc();
            push_pc();
            assume(_NOT_(match));
            symbolic m_false = last_match;
            pop_pc();
            return _ITE_VAR_(match, m_true, m_false);
        }
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    cnstr_t match = _EQ_(curr_byte, c);
    symbolic next_match;
    if (is_certain(match)) {
        next_match = (symbolic)s;
    } else if (is_certain(_NOT_(match))) {
        next_match = last_match;
    } else {
        push_pc();
        assume(match);
        symbolic m_true = (symbolic)s;
        pop_pc();
        push_pc();
        assume(_NOT_(match));
        symbolic m_false = last_match;
        pop_pc();
        next_match = _ITE_VAR_(match, m_true, m_false);
    }

    symbolic r = strrchr_rec(s + 1, c, next_match, g);
    pop_pc();

    return _ITE_VAR_(end, (symbolic)0, r); // This return is handled by base case logically but required by skeleton
}

// Re-implementing skeleton more strictly to handle the do-while nature
static symbolic strrchr_step(const char *s, symbolic c, symbolic last_match, cnstr_t guard) {
    symbolic curr_byte = (symbolic)(unsigned long)*(const unsigned char *)s;
    
    cnstr_t match = _EQ_(curr_byte, c);
    symbolic current_match;
    if (is_certain(match)) {
        current_match = (symbolic)s;
    } else if (is_certain(_NOT_(match))) {
        current_match = last_match;
    } else {
        push_pc(); assume(match); symbolic m_true = (symbolic)s; pop_pc();
        push_pc(); assume(_NOT_(match)); symbolic m_false = last_match; pop_pc();
        current_match = _ITE_VAR_(match, m_true, m_false);
    }

    cnstr_t end = _EQ_(curr_byte, 0);
    if (is_certain(end)) {
        return current_match;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    symbolic r = strrchr_step(s + 1, c, current_match, g);
    pop_pc();

    return _ITE_VAR_(end, current_match, r);
}

char *strrchr(const char *s, int c) {
    symbolic sym_c = (symbolic)(unsigned char)c;
    return (char *)strrchr_step(s, sym_c, (symbolic)0, TRUE);
}
