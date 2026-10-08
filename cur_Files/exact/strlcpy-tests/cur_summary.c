#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlcpy_rec(char *dst, const char *src, size_t n, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)src;
    cnstr_t end = _EQ_(c, 0);

    cond_write(dst, c, guard);

    if (is_certain(end)) {
        return (symbolic)0;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    symbolic res;
    if (is_certain(_EQ_(n, 0))) {
        res = (symbolic)1;
        // When n == 0, dst does not advance, but src does.
        // The loop in the original code: if (!n) { dst = dummy; } else { --n; }
        // and inside while: if (n) { --n; ++dst; }
        // This means if n was 0 initially, dst never advances.
        // If n > 0, it advances until n reaches 0.
        // Our recursion handles the logic: only recurse on src.
        push_pc();
        assume(_NOT_(end)); 
        // To avoid infinite loop, we must advance src.
        // But wait, the Rule 2 skeleton requires the end condition.
        // The loop stops ONLY when *src == 0.
        // We need a helper that tracks the source length.
        pop_pc();
    }
    pop_pc();
    return res;
}

// Redefining to fit Rule 2 skeleton properly
static symbolic strlcpy_step(char *dst, const char *src, size_t n, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)src;
    cnstr_t end = _EQ_(c, 0);

    cond_write(dst, c, guard);

    if (is_certain(end)) {
        return (symbolic)0;
    }

    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));

    symbolic next_len;
    cnstr_t n_pos = _GT_(n, 0);
    
    if (is_certain(n_pos)) {
        next_len = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_step(dst + 1, src + 1, n - 1, g));
    } else if (is_certain(_NOT_(n_pos))) {
        next_len = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_step(dst, src + 1, n, g));
    } else {
        push_pc();
        assume(n_pos);
        symbolic a = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_step(dst + 1, src + 1, n - 1, _AND_(g, n_pos)));
        pop_pc();
        push_pc();
        assume(_NOT_(n_pos));
        symbolic b = (symbolic)((unsigned long)1 + (unsigned long)strlcpy_step(dst, src + 1, n, _AND_(g, _NOT_(n_pos))));
        pop_pc();
        next_len = _ITE_VAR_(n_pos, a, b);
    }

    pop_pc();
    return _ITE_VAR_(end, (symbolic)0, next_len);
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    cnstr_t n_zero = _EQ_(n, 0);
    symbolic res;

    if (is_certain(n_zero)) {
        // dst = dummy; while(*dst = *src != 0) { ++src; }
        // essentially strlen(src)
        res = (symbolic)strlcpy_step((char *)0, src, 0, TRUE); 
    } else if (is_certain(_NOT_(n_zero))) {
        res = (symbolic)strlcpy_step(dst, src, n - 1, TRUE);
    } else {
        push_pc();
        assume(n_zero);
        symbolic a = (symbolic)strlcpy_step((char *)0, src, 0, n_zero);
        pop_pc();
        push_pc();
        assume(_NOT_(n_zero));
        symbolic b = (symbolic)strlcpy_step(dst, src, n - 1, _NOT_(n_zero));
        pop_pc();
        res = _ITE_VAR_(n_zero, a, b);
    }

    return (size_t)(unsigned long)res;
}
