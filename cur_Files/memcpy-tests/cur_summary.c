#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    allocd(dest, n);
    allocd(src, n);

    for (size_t i = 0; i < n; i++) {
        symbolic val = s[i];
        cond_write(d + i, val, TRUE);
    }

    return dest;
}
