#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    size_t i = 0;

    while (i < n) {
        allocd(d + i, 1);
        allocd(s + i, 1);
        d[i] = s[i];
        i++;
    }

    return dest;
}
