#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;

    if (n == 0) {
        return dest;
    }

    allocd(d, n);
    allocd(s, n);

    for (size_t i = 0; i < n; i++) {
        symbolic val = s[i];
        cond_write(d + i, val, TRUE);
    }

    return dest;
}
