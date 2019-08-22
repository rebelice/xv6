#include <inc/types.h>
#include <inc/x86.h>
#include <inc/mmu.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/error.h>
#include <inc/assert.h>

#include <kern/vm.h>
#include <kern/kalloc.h>
#include <kern/console.h>

// Defined by kern/kernel.ld.
extern char data[];
// Kernel's page table directory.
pde_t *kpgdir;

// Given 'pgdir', a pointer to a page directory, pgdir_walk returns
// a pointer to the page table entry (PTE) for linear address 'va'.
// This requires walking the two-level page table structure.
//
// The relevant page table page might not exist yet.
// If this is true, and alloc == false, then pgdir_walk returns NULL.
// Otherwise, pgdir_walk allocates a new page table page with kalloc.
// 		- If the allocation fails, pgdir_walk returns NULL.
// 		- Otherwise, the new page is cleared, and pgdir_walk returns
// a pointer into the new page table page.
//
// Hint 1: the x86 MMU checks permission bits in both the page directory
// and the page table, so it's safe to leave permissions in the page
// directory more permissive than strictly necessary.
//
// Hint 2: look at inc/mmu.h for useful macros that manipulate page
// table and page directory entries.
//
static pte_t *
pgdir_walk(pde_t *pgdir, const void *va, int32_t alloc)
{
	// TODO: Fill this function in
	return NULL;
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might **NOT**
// be page-aligned.
// Use permission bits perm|PTE_P for the entries.
//
// Hint: the TA solution uses pgdir_walk
static int
map_region(pde_t *pgdir, void *va, uint32_t size, uint32_t pa, int32_t perm)
{
	// TODO: Fill this function in
	return 0;
}

// This table defines the kernel's mappings, which are present in
// every process's page table.
// The example code here marks all physical as writable.
// However this is not truly the case.
// kvm_init() should set up page table like this:
//
//	KERNBASE..KERNBASE+EXTPHYSMEM: mapped to 0..EXTPHYSMEM
// 									(for I/O space)
// 	KERNBASE+EXTPHYSMEM..data: mapped to EXTPHYSMEM..V2P(data)
//					for the kernel's instructions and r/o data
// 	data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//					rw data + free physical memory
//  DEVSPACE..0: mapped direct (devices such as ioapic)
static struct kmap {
	void *virt;
	uint32_t phys_start;
	uint32_t phys_end;
	int perm;
} kmap[] = {
	// TODO: Modify the code to reflect the above.
	{ (void *)KERNBASE, 0, PHYSTOP, PTE_W},
};

// Set up kernel part of a page table.
// Return a pointer of the page table.
// Return 0 on failure.
//
// In general, you need to do the following things:
// 		1. kalloc() memory for page table;
// 		2. use map_region() to create corresponding PTEs
//			for each item of kmap[];
//
// Hint: You may need ARRAY_SIZE.
pde_t *
kvm_init(void)
{
	// TODO: your code here
	return 0;
}

// Switch h/w page table register to the kernel-only page table.
void
kvm_switch(void)
{
	lcr3(V2P(kpgdir)); // switch to the kernel page table
}

// Allocate one page table for the machine for the kernel address
// space.
void
vm_init(void)
{
	kpgdir = kvm_init();
	if (kpgdir == 0)
		panic("vm_init: failure");
	kvm_switch();
}

// Free a page table.
//
// Hint: You need to free all existing PTEs for this pgdir.
void
vm_free(pde_t *pgdir)
{
	// TODO: your code here
}