#ifndef KERN_SYSCALL_H
#define KERN_SYSCALL_H

#include <inc/syscall.h>

int32_t syscall(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

#endif /* !KERN_SYSCALL_H */
