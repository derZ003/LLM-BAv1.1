/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

.text
.globl _start
.type _start,@function

_start:
    /* rtld_fini */
    or      $r9,  $r4,  $r0

    /* main */
    la.global $r4, main

    /* argc */
    ld.d    $r5,  $r3, 0

    /* argv */
    addi.d  $r6,  $r3, 8

    /* init/fini */
    move    $r7,  $r0
    move    $r8,  $r0

    /* stack_end */
    or      $r10, $r3, $r0

    bl      __uClibc_main
