void *concrete_memccpy(void *s1, const void *s2, int c, size_t n)
{
  register char *r1 = s1;
  register const char *r2 = s2;
  while ((n--) && (((unsigned char) (*(r1++) = *(r2++))) != ((unsigned char) c)))
    ;

  return (n == ((size_t) (-1))) ? (NULL) : (r1);
}

