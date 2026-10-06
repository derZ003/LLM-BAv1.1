/*
 * Copyright (C) 2026 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

    .section .init,"ax",@progbits
    ld.d    $ra,$sp,8
    addi.d  $sp,$sp,16
    jr      $ra

    .section .fini,"ax",@progbits
    ld.d    $ra,$sp,8
    addi.d  $sp,$sp,16
    jr      $ra
