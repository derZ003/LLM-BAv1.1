/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Assembly macros for LoongArch.
   Copyright (C) 2022-2026 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/regdef.h>

#define INTERNAL_SYSCALL_NCS(number, err, nr, args...) \
  internal_syscall##nr (number, err, args)

#define internal_syscall0(number, err, dummy...) \
  ({ \
    long int _sys_result; \
\
    { \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0"); \
      __asm__ volatile ("syscall 0\n\t" \
		       : "=r"(__a0) \
		       : "r"(__a7) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall1(number, err, arg0) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall2(number, err, arg0, arg1) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall3(number, err, arg0, arg1, arg2) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      long int _arg2 = (long int) (arg2); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      register long int __a2 __asm__ ("$a2") = _arg2; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1), "r"(__a2) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall4(number, err, arg0, arg1, arg2, arg3) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      long int _arg2 = (long int) (arg2); \
      long int _arg3 = (long int) (arg3); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      register long int __a2 __asm__ ("$a2") = _arg2; \
      register long int __a3 __asm__ ("$a3") = _arg3; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall5(number, err, arg0, arg1, arg2, arg3, arg4) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      long int _arg2 = (long int) (arg2); \
      long int _arg3 = (long int) (arg3); \
      long int _arg4 = (long int) (arg4); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      register long int __a2 __asm__ ("$a2") = _arg2; \
      register long int __a3 __asm__ ("$a3") = _arg3; \
      register long int __a4 __asm__ ("$a4") = _arg4; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1), "r"(__a2), \
			 "r"(__a3), "r"(__a4) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall6(number, err, arg0, arg1, arg2, arg3, arg4, arg5) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      long int _arg2 = (long int) (arg2); \
      long int _arg3 = (long int) (arg3); \
      long int _arg4 = (long int) (arg4); \
      long int _arg5 = (long int) (arg5); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      register long int __a2 __asm__ ("$a2") = _arg2; \
      register long int __a3 __asm__ ("$a3") = _arg3; \
      register long int __a4 __asm__ ("$a4") = _arg4; \
      register long int __a5 __asm__ ("$a5") = _arg5; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3), \
			 "r"(__a4), "r"(__a5) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define internal_syscall7(number, err, arg0, arg1, arg2, arg3, arg4, arg5, arg6) \
  ({ \
    long int _sys_result; \
\
    { \
      long int _arg0 = (long int) (arg0); \
      long int _arg1 = (long int) (arg1); \
      long int _arg2 = (long int) (arg2); \
      long int _arg3 = (long int) (arg3); \
      long int _arg4 = (long int) (arg4); \
      long int _arg5 = (long int) (arg5); \
      long int _arg6 = (long int) (arg6); \
      register long int __a7 __asm__ ("$a7") = number; \
      register long int __a0 __asm__ ("$a0") = _arg0; \
      register long int __a1 __asm__ ("$a1") = _arg1; \
      register long int __a2 __asm__ ("$a2") = _arg2; \
      register long int __a3 __asm__ ("$a3") = _arg3; \
      register long int __a4 __asm__ ("$a4") = _arg4; \
      register long int __a5 __asm__ ("$a5") = _arg5; \
      register long int __a6 __asm__ ("$a6") = _arg6; \
      __asm__ volatile ("syscall 0\n\t" \
		       : "+r"(__a0) \
		       : "r"(__a7), "r"(__a1), "r"(__a2), "r"(__a3), \
			 "r"(__a4), "r"(__a5), "r"(__a6) \
		       : __SYSCALL_CLOBBERS); \
      _sys_result = __a0; \
    } \
    _sys_result; \
  })

#define __SYSCALL_CLOBBERS \
  "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "memory"

