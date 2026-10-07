static int concrete_isupper(int c)
{
	if (c >= 65 && c <= 90)
	{
		return 1;
	}
	return 0;
}

int concrete_tolower(int c)
{
	if (concrete_isupper(c))
	{
		return c | 32;
	}
	return c;
}
