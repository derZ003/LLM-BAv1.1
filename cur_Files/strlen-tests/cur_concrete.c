size_t concrete_strlen(const char *s)
{
	const char *a = s;
	for (; *s; s++);
	return s - a;
}
