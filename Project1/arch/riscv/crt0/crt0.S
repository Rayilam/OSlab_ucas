#include <asm.h>

ENTRY(_start)

    /* TODO: [p1-task3] setup C runtime environment for the user program */
    // push return address
    addi sp, sp, -8
    sd   ra, (sp)

    // clear .BSS
    lui  t0, %hi(__bss_start)
    addi t0, t0, %lo(__bss_start)
    lui  t1, %hi(_end)
    addi t1, t1, %lo(_end)
    // for (i=__bss_start; i<_end; i+=4) MEM[i]=0;
    bge  t0, t1, cleared
clear_bss:
    sw   zero, (t0)
    addi t0, t0, 4
    blt  t0, t1, clear_bss
cleared:
    /* TODO: [p1-task3] enter main function */
    call main

    /* TODO: [p1-task3] finish task and return to the kernel */
    /* NOTE: You need to replace this with new mechanism in p3-task2! */
    ld   ra, (sp)
    addi sp, sp, 8
    jr    ra

// while(1) loop, unreachable here
/*
loop:
    wfi
    j loop
*/

END(_start)
