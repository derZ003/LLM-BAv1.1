/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* syscall error handlers
   Copyright (C) 2022-2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sys/asm.h>

#define L(label) .L##label

ENTRY (__syscall_error)
/* Fall through to __syscall_set_errno */
END (__syscall_error)

/* Non-standard calling convention: argument in a0, return address in t0,
   and clobber only t1.
 */
ENTRY (__syscall_set_errno)

/* We got here because a0 < 0, but only codes in the range [-4095, -1]
   represent errors.  Otherwise, just return the result normally.
 */
	LI		t1, -4096
	bgeu		t1, a0, L (out)
	sub.w		a0, zero, a0
	st.w		a0, t1, 0
	LI		a0, -1
L (out):
	ret
END (__syscall_set_errno)
