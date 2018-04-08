#include "bionic_asm.h"

.text
    .globl my_openat
    .type my_openat,function
my_openat:
    .cfi_startproc
    mov ip, r7
    .cfi_register r7, ip
    ldr r7, =__NR_openat
    swi #0
    mov r7, ip
    .cfi_restore r7
    cmn r0, #(4095 + 1)
    bxls lr
    neg r0, r0
    b __set_errno_internal
    .cfi_endproc
    .size my_openat, .-my_openat;

.text
    .globl my_read
    .type my_read,function
my_read:
    .cfi_startproc
    mov     ip, r7
    .cfi_register r7, ip
    ldr     r7, =__NR_read
    swi     #0
    mov     r7, ip
    .cfi_restore r7
    cmn     r0, #(MAX_ERRNO + 1)
    bxls    lr
    neg     r0, r0
    b       __set_errno_internal
    .cfi_endproc
    .size my_read, .-my_read;