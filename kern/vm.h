#ifndef KERN_VM_H
#define KERN_VM_H

#include <inc/memlayout.h>

void vm_init(void);
pde_t *kvm_init(void);
void kvm_switch(void);
void vm_free(pde_t *);

void seg_init(void);
void region_alloc(struct proc *p, void *va, size_t len);
void uvm_switch(struct proc *p);

void pushcli(void);
void popcli(void);

#endif /* !KERN_VM_H */