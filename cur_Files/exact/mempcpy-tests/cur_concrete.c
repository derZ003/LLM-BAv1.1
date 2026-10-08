void *concrete_mempcpy(void *s1, const void *s2, size_t n)
{
  register char *r1 = s1;
  register const char *r2 = s2;
  while (n)
  {
    *(r1++) = *(r2++);
    --n;
  }

  return r1;
}

