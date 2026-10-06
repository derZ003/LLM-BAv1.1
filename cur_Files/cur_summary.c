#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void sym_memcpy_loop(unsigned char *d, const unsigned char *s, size_t n) {
    if (n == 0) return;
    
    symbolic val = *s;
    cond_write(d, val, TRUE);
    
    sym_memcpy_loop(d + 1, s + 1, n - 1);
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    if (dest == NULL || src == NULL) {
        return dest;
    }
    
    allocd(dest, n);
    allocd(src, n);
    
    sym_memcpy_loop((unsigned char *)dest, (const unsigned char *)src, n);
    
    return dest;
}
