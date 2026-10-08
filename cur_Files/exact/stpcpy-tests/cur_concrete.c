char *concrete_stpcpy(register char *s1, const char *s2)
{
  while ((*(s1++) = *(s2++)) != 0)
    ;

  return s1 - 1;
}

