#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static symbolic strlen_rec(const char *s, size_t offset, cnstr_t guard) {
    symbolic c = (symbolic)(unsigned long)*(const unsigned char *)s;
    cnstr_t end = _EQ_(c, 0);

    if (is_certain(end)) {
        return (symbolic)(unsigned long)offset;
    }

    push_pc();
    assume(_NOT_(end));
    
    /* Recurse: advance pointer and increment offset */
    symbolic r = strlen_rec((const char *)((unsigned long)s + 1), 
                            (size_t)((unsigned long)offset + 1), 
                            _AND_(guard, _NOT_(end)));
    pop_pc();

    return _ITE_VAR_(end, (symbolic)(unsigned long)offset, r);
}

size_t strlen(const char *s) {
    return (size_t)(unsigned long)strlen_rec(s, 0, TRUE);
}
