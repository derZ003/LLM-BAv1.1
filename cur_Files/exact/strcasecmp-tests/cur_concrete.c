static int concrete_tolower(int c)
{
	if (c >= 65 && c <= 90)
	{
		return c + 32;
	}
	return c;
}

int concrete_strcasecmp(const char *_l, const char *_r)
{
	const unsigned char *l = (const unsigned char *)_l;
	const unsigned char *r = (const unsigned char *)_r;
	while (*l != 0 && *r != 0 && (*l == *r || concrete_tolower(*l) == concrete_tolower(*r)))
	{
		l++;
		r++;
	}
	return concrete_tolower(*l) - concrete_tolower(*r);
}
