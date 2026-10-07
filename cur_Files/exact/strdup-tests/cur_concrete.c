static size_t concrete_strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0')
    {
        len++;
    }
    return len;
}

static void *concrete_memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

extern void *malloc(size_t size);

char *concrete_strdup(const char *s)
{
    size_t l = concrete_strlen(s);
    char *d = (char *)malloc(l + 1);
    if (!d)
    {
        return NULL;
    }
    return (char *)concrete_memcpy(d, s, l + 1);
}
