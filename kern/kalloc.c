#include <inc/types.h>
#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/error.h>
#include <inc/assert.h>
#include <inc/string.h>

#include <kern/kalloc.h>
#include <kern/console.h>

extern char end[];

struct run {
	struct run *next;
};

struct {
	struct run *free_list;
} kmem;

void
kfree(char *v)
{
	struct run *r;

	if ((uint32_t)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		panic("kfree");

	memset(v, 1, PGSIZE);

	r = (struct run *)v;
	r->next = kmem.free_list;
	kmem.free_list = r;
}

void
free_rage(void *vstart, void *vend)
{
	char *p;
	p = ROUNDUP((char *)vstart, PGSIZE);
	for (; p + PGSIZE <= (char *)vend; p += PGSIZE)
		kfree(p);
}

void
boot_alloc_init(void)
{
	free_rage((void *)end, P2V(4*1024*1024));
	check_free_list();
}

void
alloc_init(void)
{
	free_rage(P2V(4*1024*1024), P2V(PHYSTOP));
}

char *
kalloc(void)
{
	struct run *r;

	r = kmem.free_list;
	if (r)
		kmem.free_list = r->next;
	return (char *)r;
}

// --------------------------------------------------------------
// Checking functions.
// --------------------------------------------------------------

//
// Check that the pages on the kmem.free_list are reasonable.
//
void
check_free_list(void)
{
	struct run *p;
	if (!kmem.free_list)
		panic("'kmem.free_list' is a null pointer!");

	cprintf("0x%x\n", end);

	for (p = kmem.free_list; p; p = p->next) {
		cprintf("0x%x\n", p);
		assert((void *)p > (void *)end);
		assert((void *)p <= P2V(4*1024*1024));
	}
}