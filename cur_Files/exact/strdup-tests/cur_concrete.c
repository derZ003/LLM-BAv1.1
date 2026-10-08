void *concrete_memcpy(void *s1, const void *s2, size_t n)
{
  register char *r1 = s1;
  register const char *r2 = s2;
  while (n)
  {
    *(r1++) = *(r2++);
    --n;
  }

  return s1;
}

size_t concrete_strlen(const char *s)
{
  register const char *p;
  for (p = s; *p; p++)
    ;

  return p - s;
}

char *concrete_strdup(register const char *s1)
{
  register char *s;
  register size_t l = (concrete_strlen(s1) + 1) * (sizeof(char));
  if ((s = malloc(l)) != NULL)
  {
    concrete_memcpy(s, s1, l);
  }
  return s;
}

