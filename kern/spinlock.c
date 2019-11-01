// Mutual exclusion spin locks.

#include <inc/types.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/string.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>

// The big kernel lock
struct spinlock kernel_lock = {
#ifdef DEBUG_SPINLOCK
	.name = "kernel_lock"
#endif
};

void
__spin_initlock(struct spinlock *lk, char *name)
{
	// TODO: Your code here.
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
spin_lock(struct spinlock *lk)
{
	// The xchg is atomic.
	// It also serializes, so that reads after acquire are not
	// reordered before it. 
	// TODO: Your code here.
}

// Release the lock.
void
spin_unlock(struct spinlock *lk)
{

	// The xchg instruction is atomic (i.e. uses the "lock" prefix) with
	// respect to any other instruction which references the same memory.
	// x86 CPUs will not reorder loads/stores across locked instructions
	// (vol 3, 8.2.2). Because xchg() is implemented using asm volatile,
	// gcc will not reorder C statements across the xchg.
	// TODO: Your code here.
}
