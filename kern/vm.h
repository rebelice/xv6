#ifndef KERN_VM_H
#define KERN_VM_H

#include <inc/memlayout.h>

void kvmalloc(void);
pde_t *setupkvm(void);
void switchkvm(void);
void freevm(pde_t *);


#endif /* !KERN_VM_H */