void *concrete_memcpy(void *s1, const void *s2, size_t n)
{
    unsigned char *r1 = (unsigned char *)s1;
    const unsigned char *r2 = (const unsigned char *)s2;

    while (n != 0) {
        *r1 = *r2;
        r1++;
        r2++;
        n--;
    }

    return s1;
}
