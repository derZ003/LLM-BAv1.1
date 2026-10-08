void *concrete_memrchr(const void *s, int c, size_t n)
{
  register const unsigned char *r;
  r = ((unsigned char *) s) + ((size_t) n);
  while (n)
  {
    if ((*(--r)) == ((unsigned char) c))
    {
      return (void *) r;
    }
    --n;
  }

  return NULL;
}

