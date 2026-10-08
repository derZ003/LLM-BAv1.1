char *concrete_strncpy(char *s1, register const char *s2, size_t n)
{
  register char *s = s1;
  while (n)
  {
    if ((*s = *s2) != 0)
      s2++;
    ++s;
    --n;
  }

  return s1;
}

