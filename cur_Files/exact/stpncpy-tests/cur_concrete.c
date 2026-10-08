char *concrete_stpncpy(register char *s1, register const char *s2, size_t n)
{
  char *s = s1;
  const char *p = s2;
  while (n)
  {
    if ((*s = *s2) != 0)
      s2++;
    ++s;
    --n;
  }

  return s1 + (s2 - p);
}

