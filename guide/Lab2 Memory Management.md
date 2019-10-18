# 2 lab2 Memory Management

`Two weeks`

[toc]

## 2.1 Introduction

### 2.1.1 Objectives

In this lab, you will write the memory management code for your operating system. Memory management has two components.

The first component is a physical memory allocator for the kernel, so that the kernel can allocate memory and later free it. Your allocator will operate in units of 4096 bytes, called *pages*.  Your task will be to maintain data structures that record which physical pages are free. You will also write the routines to allocate and free pages of memory.

The second component of memory management is *virtual memory*, which maps the virtual addresses used by kernel and user software to addresses in physical memory. The x86 hardware's memory management unit (MMU) performs the mapping when instructions use memory, consulting a set of page tables. You will modify codes to set up the MMU's page tables according to a specification we provide.

### 2.1.2 Materials

In this and future labs you will progressively build up your kernel. We will also provide you with some additional source. To fetch that source, use Git to commit changes you've made since handing in lab 1 (if any), fetch the latest version of the course repository, and then merge the lab2 branch:

```git
git pull
git merge origin/lab2
```

### 2.1.2 Lab requirements

In this lab and subsequent labs, do all of the regular exercises described in the lab. Additionally, write up brief answers to the questions posed in the lab and a short (e.g., one or two paragraph) description of what you did. Place the write-up in a file in the top level of your lab directory before handing in your work.

There may be some challenge problems.  You can choose the problem you like to solve. (Some challenge problems are more challenging than others, of course!) There is no time limit for the challenge problems. There are additional scores for completing the challenge problems. You can communicate with TA before you are ready to do a challenge problem.

## 2.2 Physical page management

The operating system must keep track of which parts of physical RAM are free. Xv6 manages the PC's physical memory with *page granularity* so that it can use the MMU to map and protect each piece of allocated memory.

You'll now write the physical page allocator. The kernel use physical page allocator to allocate and free physical memory at run-time for page tables, process user memory, kernel stacks, and pipe buffers.

Xv6 uses the physical memory between the end of the kernel and PHYSTOP for run-time allocation. It allocates and free whole 4096-byte pages at a time. It keeps track of which pages are free by threading a linked list through the pages themselves. Allocation consists of removing a page from the linked list; freeing consists of adding the freed page to the list.

There is a bootstrap problem: all of physical memory must be mapped in order for the allocator to initialize the free list, but creating a page table with those mappings involves allocating page-table pages. Xv6 solves this problem by using a separate page allocator during entry, which allocates memory juse after the end of the kernel's data segment. This allocator does not support freeing and is limited by the 4 MB mapping in the entrypgdir, but that is sufficient to allocate the first kernel page table.

### 2.2.1 Code: Physical memory allocator

The allocator's data structure is a *free list* of physical memory pages that are available for allocation. Each free page's list element is a *struct run*(kern/kalloc.c:21). Where does the allocator get the memory to hold that data structure? It store each free page's *run* structure in the free page itself, since there's nothing else stored there.

The function *i386_init*(kern/init.c:10) calls *boot_alloc_init*(kern/kalloc.c:35) and *alloc_init*(kern/kalloc.c:41) to initialize the allocator. The reason for having two calls is that for much of *i386_init* one cannot use memory above 4 megabytes. The call to *boot_alloc_init* sets up for allocation in the first 4 megabytes, and the call to *alloc_init* arranges for more memory to be allocatable. Xv6 ought to determine how much physical memory is available, but this turns out to be difficult on the x86. Instead it assumes that the machine has 240 megabytes (PHYSTOP) of physical memory, ans uses all the memory between the end of the kernel and PHYSTOP as the initial pool of free memory. For details on how to implement *boot_alloc_init* and *alloc_init*, please see the *Exercise 1* below.

Here we have provided two well-implemented functions, *free_range*(kern/kalloc.c:64) and *kfree*(kern/kalloc.c:48). *free_range* adds memory to the free list via per-page calls to *kfree*. A PTE(Page Table Entry) can only refer to a physical address that is aligned on a 4096-byte boundary (is a multiple of 4096), so *free_range* uses *ROUNDUP*(kern/kalloc.c:67) to ensure that it fress only aligned physical addresses. The allocator starts with no memory; these calls to *kfree* give it some to manage.

The allocator sometimes treats addresses as integers in order to perform arithmetic on them, and sometimes uses addresses as pointers to read and write memory; this dual use of addresses is the main reason that the allocator code is full of C type casts. The other reason is that freeing and allocation inherently change the type of the memory.

The function *kfree* begins by setting every byte in the memory being freed to the value 1. This will cause code that uses memory after freeing it (uses "dangling references") to read garbage instead of the old valid contents; hopefully that will cause such code to break faster. Then kfree casts v to a pointer to *struct run*, records the old start of the free list in *r->next*, and sets the free list qual to *r*.

## 2.3 Virtual memory

Before doing anything else, familiarize yourself with x86's protected-mode memory management architecture: namely *segmentation* and *page translation*.

### 2.3.1 Virtual, Linear, and Physical Addresses

In x86 terminology, a *virtual address* consists of a segment selector and an offset within the segment. A *linear address* is what you get after segment translation but before page translation. A *physical address* is what you finally get after both segment and page translation and what ultimately goes out on the hardware bus to your RAM.

A C pointer is the "offset" component of the virtual address. In boot/bootasm.S, we installed a Global Descriptor Table (GDT) that effectively disabled segment translation by setting all segment base addresses to 0 and limits to 0xffffffff. Hence the "selector" has no effect and the linear address always quals the offset of the virtual address.

Recall that in lab 1, we installed a simple page table so that the kernel could run at its link address of 0xf0100000, even though it is actually loaded in physical memory just above the ROM BIOS at 0x00100000. This page table mapped only 4MB of memory. In the virtual address space layout you are going to set up for xv6 in this lab, we'll expand this to map the first 240MB of physical memory starting at virtual address 0xf0000000 and to map a number of other regions of the virtual address space.

From code executing on the CPU, once we're in protected mode (which we entered first thing in boot/bootasm.S), there's no way to directly use a linear or physical address. *All* memory references are interpreted as virtual addresses and translated by the MMU, which means all pointers in C are virtual addresses.

The kernel often needs to manipulate addresses as opaque values or as integers, for example in the physical memory allocator. Sometimes these are virtual addresses, and sometimes they are physical addresses. You **MUST** be clear whether the address you are operating is a physical or virtual address.

The kernel sometimes needs to read or modify memory for which it knows only the physical address. For example, adding a mapping to a page table may require allocating physical memory to store a page directory and then initializing that memory. However, the kernel cannot bypass virtual address translation and thus cannot directly load and store to physical addresses. One reason xv6 remaps all of physical memory starting from physical address 0 at virtual address 0xf0000000 is to help the kernel read and write memory for which it knows just the physical address. In order to translate a physical address into a virtual address that the kernel can actually read and write, the kernel must add 0xf0000000 to the physical address to find its coresponding virtual address in the remapped region. You should use *P2V(pa)* to do that addition. On the contrary, use *V2P(va)* to do that subtraction.

## 2.4 Exercise

There are detailed comments in the code we provide, read them and complete the exercises below.

### 2.4.1 Exercise 1

In the file kern/kalloc.c, you must implement code for the following functions:

- boot_alloc_init()
- alloc_init()
- kalloc()

check_free_list() is a simple test code. You need to modify it so that it can make sure your physical memory allocator is correct as much as possible. And then call it in the right place.

### 2.4.2 Exercise 2

In the file kern/vm.c, you must implement code for the following functions:

- pgdir_walk()
- map_region()
- kvm_init()
- vm_free()

### 2.4.3 Exercise 3

Answer the following questions:

1. About physical allocator, why do we need to initialize it in two phases?

## 2.5 Challenge problems

1. Implement the most complete test code for kern/vm.c

## 2.6 References

1. [OSDev](https://wiki.osdev.org/Main_Page)