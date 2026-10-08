#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void strcpy_rec(char *d, const char *s, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);
    if (is_certain(end)) {
        cond_write(d, 0, guard);
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(d, c, g);
    strcpy_rec(d + 1, s + 1, g);
    pop_pc();

    /* Rule 1: Handle the case where end is undecided.
       The base case above only handles is_certain(end).
       The assume(_NOT_(end)) block handles the non-terminating path.
       We must also ensure the null-terminator is written if end is true. */
    cond_write(d, 0, _AND_(guard, end));
}

char *strcpy(char *dest, const char *src) {
    strcpy_rec(dest, src, TRUE);
    return dest;
}
