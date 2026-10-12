int concrete_memcmp(const void *s1, const void *s2, size_t n)
{
  register const unsigned char *r1 = (const unsigned char *) s1;
  register const unsigned char *r2 = (const unsigned char *) s2;
  int r = 0;
  while ((n--) && ((r = ((int) (*(r1++))) - (*(r2++))) == 0))
    ;

  return r;
}

