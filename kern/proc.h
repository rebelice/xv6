#ifndef KERN_PROC_H
#define KERN_PROC_H

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

#define NPROC 64
#define KSTACKSIZE 4096

#include <inc/types.h>
#include <inc/memlayout.h>
#include <kern/spinlock.h>

struct context {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
};

struct proc {
	pde_t *pgdir;				// Page table
	char *kstack;				// Bottom of kernel stack of this process
	enum procstate state;		// Process state
	uint32_t pid;				// Process ID
	struct proc *parent;		// Parent process
	struct trapframe *tf;		// Trapframe for current syscall
	struct context *context;	// swtch() here to return process
};

struct ptable {
	struct spinlock lock;
	struct proc proc[NPROC];
};

void proc_init(void);
void user_init(void);
void user_run(struct proc *p);
void ucode_run(void);

void exit(void);

// Without this extra macro, we couldn't pass macros like TEST to
// UCODE_LOAD because of the C pre-processor's argument prescan rule.
#define UCODE_PASTE3(x, y, z) x ## y ## z

#define UCODE_LOAD(p, x)						\
	do {								\
		extern uint8_t UCODE_PASTE3(_binary_obj_, x, _start)[];	\
		ucode_load(p, UCODE_PASTE3(_binary_obj_, x, _start));		\
	} while (0)

#endif /* !KERN_PROC_H */
