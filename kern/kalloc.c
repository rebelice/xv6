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
	struct run *freelist;
} kmem;

void
kfree(char *v)
{
	struct run *r;

	if ((uint32_t)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		panic("kfree");

	memset(v, 1, PGSIZE);

	r = (struct run *)v;
	r->next = kmem.freelist;
	kmem.freelist = r;
}

void
freerage(void *vstart, void *vend)
{
	char *p;
	p = ROUNDUP((char *)vstart, PGSIZE);
	for (; p + PGSIZE <= (char *)vend; p += PGSIZE)
		kfree(p);
}

void
kinit1(void *vstart, void *vend)
{
	freerage(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
	freerage(vstart, vend);
}

char *
kalloc(void)
{
	struct run *r;

	r = kmem.freelist;
	if (r)
		kmem.freelist = r->next;
	return (char *)r;
}