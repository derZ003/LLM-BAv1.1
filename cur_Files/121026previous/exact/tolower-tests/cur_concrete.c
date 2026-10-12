int concrete_tolower(int c)
{
  return (((unsigned int) (c - 'A')) < 26) ? (c | 0x20) : (c);
}

