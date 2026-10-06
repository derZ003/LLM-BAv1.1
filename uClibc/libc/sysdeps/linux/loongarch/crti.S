/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

    .section .init,"ax",@progbits
    .globl _init
    .type _init,@function
_init:
    addi.d  $sp,$sp,-16
    st.d    $ra,$sp,8

    .section .fini,"ax",@progbits
    .globl _fini
    .type _fini,@function
_fini:
    addi.d  $sp,$sp,-16
    st.d    $ra,$sp,8
