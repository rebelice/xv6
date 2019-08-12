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

extern char data[];
pde_t *kpgdir;

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
pgdir_walk(pde_t *pgdir, const void *va, int32_t alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
map_region(pde_t *pgdir, void *va, uint32_t size, uint32_t pa, int32_t perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)ROUNDDOWN((uint32_t)va, PGSIZE);
  last = (char*)ROUNDDOWN(((uint32_t)va) + size - 1, PGSIZE);
  for(;;){
    if((pte = pgdir_walk(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

static struct kmap {
	void *virt;
	uint32_t phys_start;
	uint32_t phys_end;
	int perm;
} kmap[] = {
	{ (void *)KERNBASE, 0, EXTPHYSMEM, PTE_W},
	{ P2V(EXTPHYSMEM), EXTPHYSMEM, V2P(data), 0},
	{ (void *)data, V2P(data), PHYSTOP, PTE_W},
	{ (void *)DEVSPACE, DEVSPACE, 0, PTE_W},
};

pde_t *
kvm_init(void)
{
	pde_t *pgdir;
	struct kmap *k;

	if ((pgdir = (pde_t *)kalloc()) == 0)
		return 0;
	memset(pgdir, 0, PGSIZE);
	if (P2V(PHYSTOP) > (void *)DEVSPACE)
		panic("PHYSTOP too high");
	for (k = kmap; k < &kmap[ARRAY_SIZE(kmap)]; k++)
		if (map_region(pgdir, k->virt, k->phys_end - k->phys_start,
					(uint32_t)k->phys_start, k->perm) < 0) {
			vm_free(pgdir);
			return 0;
		}
	return pgdir;
}

void
kvm_switch(void)
{
	lcr3(V2P(kpgdir));
}

void
vm_init(void)
{
	kpgdir = kvm_init();
	if (kpgdir == 0)
		panic("vm_init: failure");
	kvm_switch();
}

void
vm_free(pde_t *pgdir)
{
	uint32_t i;

	if (pgdir == 0)
		panic("vm_free: no pgdir");
	for (i = 0; i < NPDENTRIES; i++) {
		if (pgdir[i] & PTE_P) {
			char *v = P2V(PTE_ADDR(pgdir[i]));
			kfree(v);
		}
	}
	kfree((char *)pgdir);
}