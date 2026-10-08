int concrete_strcmp(register const char *s1, register const char *s2)
{
  int r;
  while (((r = ((int) (*((unsigned char *) s1))) - (*((unsigned char *) (s2++)))) == 0) && (*(s1++)))
    ;

  return r;
}

