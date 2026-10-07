char *concrete_strchr(register const char *s, int c)
{
  do
  {
    if ((*s) == ((char) c))
    {
      return (char *) s;
    }
  }
  while (*(s++));
  return NULL;
}

