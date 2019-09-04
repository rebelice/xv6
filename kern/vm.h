#ifndef KERN_VM_H
#define KERN_VM_H

#include <inc/memlayout.h>

void vm_init(void);
pde_t *kvm_init(void);
void kvm_switch(void);
void vm_free(pde_t *);

void seg_init(void);


#endif /* !KERN_VM_H */