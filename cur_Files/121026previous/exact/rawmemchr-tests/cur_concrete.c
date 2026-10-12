void *concrete_rawmemchr(const void *s, int c)
{
  register const unsigned char *r = s;
  while ((*r) != ((unsigned char) c))
    ++r;

  return (void *) r;
}

