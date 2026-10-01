static size_t strlen_recursive(const char *s, size_t offset) {
    symbolic current_char = *(const char *)s;
    cnstr_t is_null = _EQ_(current_char, 0);

    if (is_certain(is_null)) {
        return offset;
    } else if (is_certain(_NOT_(is_null))) {
        return strlen_recursive(s + 1, offset + 1);
    } else {
        push_pc();
        assume(is_null);
        size_t len_null = offset;
        pop_pc();

        push_pc();
        assume(_NOT_(is_null));
        size_t len_not_null = strlen_recursive(s + 1, offset + 1);
        pop_pc();

        return (size_t)_ITE_VAR_(is_null, (symbolic)len_null, (symbolic)len_not_null);
    }
}

size_t strlen(const char *s) {
    return strlen_recursive(s, 0);
}
