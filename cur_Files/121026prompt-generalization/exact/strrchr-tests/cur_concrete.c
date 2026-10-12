char *concrete_strrchr(register const char *s, int c)
{
  register const char *p;
  p = NULL;
  do
  {
    if ((*s) == ((char) c))
    {
      p = s;
    }
  }
  while (*(s++));
  return (char *) p;
}

