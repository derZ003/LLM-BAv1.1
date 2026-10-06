/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* The clone syscall wrapper.
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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sys/asm.h>
#include <sys/syscall.h>
#define _ERRNO_H  1
#include <bits/errno.h>

#define L(label) .L##label
/* Align reg to 2^n.  Used in assembly.  */
#define REG_ALIGN_ASM(reg, n) bstrins.d reg, zero, (n-1), 0
#define STACK_ALIGN 16

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
   void *parent_tidptr, void *tls, void *child_tidptr) */

ENTRY (__clone)

	/* Align stack to 16.  */
	REG_ALIGN_ASM (a1, 4)

	/* Sanity check arguments.  */
	beq		a0, zero, L (invalid) /* No NULL function pointers.  */
	beq		a1, zero, L (invalid) /* No NULL stack pointers.  */

	ADDI 		a1, a1, -STACK_ALIGN /* Reserve argument save space.  */
	REG_S		a0, a1, 0   /* Save function pointer.  */
	REG_S		a3, a1, SZREG   /* Save argument pointer.  */

	/* The syscall expects the args to be in different slots.  */
	or		a0, a2, zero
	or		a2, a4, zero
	or		a3, a6, zero
	or		a4, a5, zero

	/* Do the system call.  */
	LI		a7,__NR_clone
	syscall		0

	blt		a0, zero ,L (error)
	beq		a0, zero, L (thread_start)

	/* Successful return from the parent.  */
	ret

L (invalid):
	LI		a0, -EINVAL

	/* Something bad happened -- no child created.  */
L (error):
	b		__syscall_error

END (__clone)

/* Load up the arguments to the function.  Put this block of code in
   its own function so that we can terminate the stack trace with our
   debug info.  */
ENTRY (__thread_start)
L (thread_start):

	/* Restore the arg for user's function.  */
	REG_L		a1, sp, 0   /* Function pointer.  */
	REG_L		a0, sp, SZREG   /* Argument pointer.  */

	/* Call the user's function.  */
	jirl		ra, a1, 0

	/* Call exit with the function's return value.  */
	LI		a7, __NR_exit
	syscall		0

	END (__thread_start)

libc_hidden_def (__clone)
weak_alias (__clone, clone)
