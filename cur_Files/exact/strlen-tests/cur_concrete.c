size_t concrete_strlen(const char *s)
{
  register const char *p;
  for (p = s; *p; p++)
    ;

  return p - s;
}

