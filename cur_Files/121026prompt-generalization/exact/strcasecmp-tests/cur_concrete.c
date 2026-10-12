int concrete_tolower(int c)
{
  return (((unsigned int) (c - 'A')) < 26) ? (c | 0x20) : (c);
}

int concrete_strcasecmp(register const char *s1, register const char *s2)
{
  int r = 0;
  while (((s1 == s2) || (!(r = ((int) concrete_tolower(*((unsigned char *) s1))) - concrete_tolower(*((unsigned char *) s2))))) && ((++s2, *(s1++))))
    ;

  return r;
}

