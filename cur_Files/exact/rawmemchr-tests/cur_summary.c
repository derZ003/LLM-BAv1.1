#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic rec_rawmemchr(const unsigned char *s, unsigned char c) {
    symbolic curr_byte = (symbolic)(unsigned long)*s;
    cnstr_t end = _EQ_(curr_byte, (symbolic)c);

    if (is_certain(end)) {
        return (symbolic)s;
    }

    push_pc();
    assume(_NOT_(end));
    symbolic r = rec_rawmemchr(s + 1, c);
    pop_pc();

    return _ITE_VAR_(end, (symbolic)s, r);
}

void *rawmemchr(const void *s, int c) {
    return (void *)rec_rawmemchr((const unsigned char *)s, (unsigned char)c);
}
