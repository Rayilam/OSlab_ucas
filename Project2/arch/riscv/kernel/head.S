/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
    // check riscv.lds for __bss_start and __BSS_END__
  //for(;__bss_start<__BSS_END__;start+4)
  lui  t0, %hi(__bss_start)
  addi t0, t0, %lo(__bss_start)
  lui  t1, %hi(__BSS_END__)
  addi t1, t1, %lo(__BSS_END__)
  //if __bss_start >= __BSS_END__ goto cleared
  bge  t0, t1, cleared
clear_bss:
  sw zero,(t0)
  addi t0,t0,4 //++
  //if __bss_start<__BSS_END__ goto clear_bss
  blt t0,t1,clear_bss
cleared:
  /* TODO: [p1-task2] setup C environment */
  lui sp,%hi(KERNEL_STACK)
  addi sp,sp,%lo(KERNEL_STACK)
  call main

loop:
  wfi
  j loop

END(_start)
