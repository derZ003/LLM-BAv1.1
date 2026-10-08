#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static void bzero_fill(unsigned char *p, size_t n, cnstr_t guard) {
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {
        return;
    }
    cnstr_t g = _AND_(guard, _NOT_(n_zero));
    cond_write(p, 0, g);
    push_pc();
    assume(_NOT_(n_zero));
    bzero_fill(p + 1, n - 1, g);
    pop_pc();
}

void bzero(void *s, size_t n) {
    bzero_fill((unsigned char *)s, n, TRUE);
}

int bzero_w(void *s, size_t n)
{
  bzero(s, n);
  return 0;
}
