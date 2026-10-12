char *concrete_strchrnul(register const char *s, int c)
{
  --s;
  while ((*(++s)) && ((*s) != ((char) c)))
    ;

  return (char *) s;
}

