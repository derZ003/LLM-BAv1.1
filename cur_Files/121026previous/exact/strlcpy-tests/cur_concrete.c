size_t concrete_strlcpy(register char *dst, register const char *src, size_t n)
{
  const char *src0 = src;
  char dummy[1];
  if (!n)
  {
    dst = dummy;
  }
  else
  {
    --n;
  }
  while ((*dst = *src) != 0)
  {
    if (n)
    {
      --n;
      ++dst;
    }
    ++src;
  }

  return src - src0;
}

