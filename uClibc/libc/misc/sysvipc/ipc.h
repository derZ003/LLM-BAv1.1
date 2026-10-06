#ifndef IPC_H
#define IPC_H
#include <syscall.h>
#include <bits/kernel-features.h>
#include <bits/wordsize.h>
#ifdef __mips__
#include <sgidefs.h>
#endif

#ifndef __ARCH_HAS_DEPRECATED_SYSCALLS__
#  define __IPC_64	0x0
#elif defined __mips__ && _MIPS_SIM != _ABIO32
/* n32/n64 route the *ctl syscalls through sys_old_*ctl (n32: the compat_
   variants), which call ipc_parse_version() and so strip IPC_64 out of cmd to
   select the layout on every kernel version.  The bit must therefore always be
   set, otherwise the kernel returns the ancient struct and e.g. sem_nsems
   comes back as 0.  */
#  define __IPC_64	0x100
#elif defined __m68k__ || defined __i386__ || defined __mips__
/* 5.1+ uses the direct *ctl syscalls, which (unlike ipc()) do not strip
   IPC_64 -- passing it would make them fail with EINVAL.  mips o32 got them
   as 394/396/402 and belongs here; n32/n64 are handled above.  */
# if __LINUX_KERNEL_VERSION < 0x050100
#  define __IPC_64      0x100
# else
#  define __IPC_64      0x0
# endif
#elif defined __hppa__
/* parisc has no ARCH_WANT_IPC_PARSE_VERSION: the kernel never strips
   IPC_64, so passing it makes semctl & co. fail with EINVAL.  */
#  define __IPC_64	0x0
#else
# if __WORDSIZE == 32 || defined __alpha__
#  define __IPC_64	0x100
# else
#  define __IPC_64	0x0
# endif
#endif

#ifdef __NR_ipc

/* The actual system call: all functions are multiplexed by this.  */
extern int __syscall_ipc (unsigned int __call, long __first, long __second,
					  long __third, void *__ptr, void *__fifth) attribute_hidden;


/* The codes for the functions to use the multiplexer `__syscall_ipc'.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_semtimedop 4
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24

#endif

#endif							/* IPC_H */
