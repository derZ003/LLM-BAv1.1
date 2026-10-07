int concrete_strncmp(register const char *s1, register const char *s2, size_t n)
{
  int r = 0;
  while (((n--) && ((r = ((int) (*((unsigned char *) s1))) - (*((unsigned char *) (s2++)))) == 0)) && (*(s1++)))
    ;

  return r;
}

