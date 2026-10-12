size_t concrete_strnlen(const char *s, size_t max)
{
  register const char *p = s;
  while (max && (*p))
  {
    ++p;
    --max;
  }

  return p - s;
}

