#include <inc/types.h>
#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/x86.h>
#include <inc/traps.h>

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint32_t vectors[]; // in vectors.S: array of 256 entry pointers

// Initialize the interrupt descriptor table.
void
trap_init(void)
{
	// Your code here.
	//
	// Hints:
	// 1. The macro SETGATE in inc/mmu.h should help you, as well as the
	// T_* defines in inc/traps.h;
	// 2. T_SYSCALL is different from the others.
}

void
trap(struct trapframe *tf)
{
	// You don't need to implement this function now, but you can write
	// some code for debugging.
}