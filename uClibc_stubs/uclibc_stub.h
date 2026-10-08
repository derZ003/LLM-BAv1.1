#define __USE_GNU
#define __USE_BSD
#define __UCLIBC_SUSV3_LEGACY__
#define libc_hidden_def(x)
#define libc_hidden_weak(x)
#define libc_hidden_proto(x)
#define libc_hidden_data_def(x)
#define strong_alias(a,b)
#define weak_alias(a,b)
#define attribute_hidden
#define __restrict
#define __attribute__(x)
#define __XL_NPP(N) N
#define __LOCALE_PARAM
#define __LOCALE_ARG
#define __C_isupper(c) (((unsigned int)((c) - 'A')) < 26)
#define __C_tolower(c) (__C_isupper(c) ? ((c) | 0x20) : (c))
