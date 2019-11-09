#include <inc/types.h>
#include <inc/memlayout.h>
#include <inc/traps.h>
#include <inc/mmu.h>
#include <inc/elf.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/kalloc.h>
#include <kern/proc.h>
#include <kern/vm.h>
#include <kern/trap.h>
#include <kern/cpu.h>

struct ptable ptable;

uint32_t nextpid = 1;
extern pde_t *kpgdir;
extern void forkret(void);
extern void trapret(void);
void swtch(struct context **, struct context*);


//
// Initialize something about process, such as ptable.lock
//
void
proc_init(void)
{
	// TODO: your code here
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0
static struct proc*
proc_alloc(void)
{
	// TODO: your code here
	//
	// Following things you need to do:
	// - set state and pid.
	// - allocate kernel stack.
	// - leave room for trap frame in kernel stack.
	// - Set up new context to start executing at forkret, which returns to trapret.


}

//
// Set up the initial program binary, stack, and processor flags
// for a user process.
// This function is ONLY called during kernel initialization,
// before running the first user-mode process.
//
// This function loads all loadable segments from the ELF binary image
// into the environment's user memory, starting at the appropriate
// virtual addresses indicated in the ELF program header.
// At the same time it clears to zero any portions of these segments
// that are marked in the program header as being mapped
// but not actually present in the ELF file - i.e., the program's bss section.
//
// All this is very similar to what our boot loader does, except the boot
// loader also needs to read the code from disk.  Take a look at
// boot/bootmain.c to get ideas.
//
// Finally, this function maps one page for the program's initial stack.
//
// load_icode panics if it encounters problems.
//  - How might load_icode fail?  What might be wrong with the given input?
//
static void
ucode_load(struct proc *p, uint8_t *binary) {
	// Hints:
	//  Load each program segment into virtual memory
	//  at the address specified in the ELF segment header.
	//  You should only load segments with ph->p_type == ELF_PROG_LOAD.
	//  Each segment's virtual address can be found in ph->p_va
	//  and its size in memory can be found in ph->p_memsz.
	//  The ph->p_filesz bytes from the ELF binary, starting at
	//  'binary + ph->p_offset', should be copied to virtual address
	//  ph->p_va.  Any remaining memory bytes should be cleared to zero.
	//  (The ELF header should have ph->p_filesz <= ph->p_memsz.)
	//  Use functions from the previous lab to allocate and map pages.
	//
	//  All page protection bits should be user read/write for now.
	//  ELF segments are not necessarily page-aligned, but you can
	//  assume for this function that no two segments will touch
	//  the same virtual page.
	//
	//  You may find a function like region_alloc useful.
	//
	//  Loading the segments is much simpler if you can move data
	//  directly into the virtual addresses stored in the ELF binary.
	//  So which page directory should be in force during
	//  this function?
	//
	//  You must also do something with the program's entry point,
	//  to make sure that the environment starts executing there.
	//  What? 

	// TODO: Your code here.

	// Now map one page for the program's initial stack
	// at virtual address USTACKTOP - PGSIZE.

	// TODO: Your code here.
}

//
// Allocates a new proc with proc_alloc, loads the user_hello elf
// binary into it with UCODE_LOAD.
// This function is ONLY called during kernel initialization,
// before running the first user-mode process.
// The new proc's parent ID is set to 0.
//
void
user_init(void)
{
	// TODO: your code here.
}

//
// Context switch from scheduler to first proc.
//
// This function does not return.
//
void
ucode_run(void)
{
	// TODO: your code here
	//
	// Hints:
	// - you may need sti() and cli()
	// - you may need uvm_switch(), swtch() and kvm_switch()
}

struct proc*
thisproc(void) {
	struct proc *p;
	pushcli();
	p = thiscpu->proc;
	popcli();
	return p;
}

//
// Context switch from thisproc to scheduler.
//
void
sched(void)
{
	// TODO: your code here.
}


void
forkret(void)
{
	// Return to "caller", actually trapret (see proc_alloc)
	// That means the first proc starts here.
	// When it returns from forkret, it need to return to trapret.
	// TODO: your code here.

}

void
exit(void)
{
	// sys_exit() call to here.
	// TODO: your code here.
}