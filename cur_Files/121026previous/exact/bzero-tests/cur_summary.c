#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void bzero_rec(unsigned char *p, size_t n, cnstr_t guard) {
    cnstr_t end = _EQ_(n, 0);
    if (is_certain(end)) {
        return;
    }
    push_pc();
    assume(_NOT_(end));
    cnstr_t g = _AND_(guard, _NOT_(end));
    cond_write(p, 0, g);
    bzero_rec(p + 1, n - 1, g);
    pop_pc();
}

void bzero(void *s, size_t n) {
    bzero_rec((unsigned char *)s, n, TRUE);
}

int bzero_w(void *s, size_t n)
{
  bzero(s, n);
  return 0;
}
