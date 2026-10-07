void *concrete_memchr(const void *s, int c, size_t n)
{
  register const unsigned char *r = (const unsigned char *) s;
  while (n)
  {
    if ((*r) == ((unsigned char) c))
    {
      return (void *) r;
    }
    ++r;
    --n;
  }

  return NULL;
}

