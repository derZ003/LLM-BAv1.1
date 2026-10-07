void *concrete_memset(void *s, int c, size_t n)
{
  register unsigned char *p = (unsigned char *) s;
  while (n)
  {
    *(p++) = (unsigned char) c;
    --n;
  }

  return s;
}

