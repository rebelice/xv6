# 1 lab1 Booting

`One week`

[toc]

## 1.1 Introduction

### 1.1.1 Objectives

From now on, we will officially enter our project. In this lab you will know how to boot on a PC.

### 1.1.1 Materials

We recommend that you create a new branch instead of completing the project on an existing branch. The following will create a new branch based on branch `lab1`. You can name the new branch at will. Make sure you are in the root directory of the framework code.

```git
git checkout lab1
git checkout -b dev
```

## 1.2 x86

Our labs are all aimed at the x86 architecture. Everything you want to know about x86 is in the `Intel 64 and IA-32 Architectures Software Developer's Manual`. You don't have to read them now, but you'll almost certainly want to refer to some of this material when reading and writing x86-related codes.

### 1.2.1 x86 assembly

Our project is mainly composed of C language and x86 assembly. For GNU, we use the x86 assembly of the AT&T syntax. We do recommend reading the [Brennan's Guide to Inline Assembly](http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html). It gives a good and quite brief description of the AT&T assembly syntax we'll using with the GNU assembler.

## 1.3 The boot loader

We have introduced the BIOS in lab0. When an x86 PC boots,  it starts executing the BIOS. And then, the BIOS transfers control to code loaded from the boot sector. The boot sector contains the boot loader: instructions that load the kernel into memory. The BIOS loads the boot sector at memory address 0x7c00 and then jumps to that address. When the boot loader begins executing, the processor is in real mode, and the loader's job is to put the processor in a more modern operating mode, to load the xv6 kernel from disk into memory, and then to transfer control to the kernel.

We will use the conventional hard drive boot mechanism, which means that our boot loader must fit into a measly 512 bytes(actually 510 bytes, there must be two magic numbers). The boot loader consists of one assembly language source file, `boot/bootasm.S`, and one C source file, `boot/bootmain.c`. Look through these source files carefully and make sure you understand what's going on. The boot loader must perform two main functions:
    1. First, the boot loader switches the processor from read mode to 32-bit protected mode, because it is only in this mode that software can access all the memory above 1MB in the processor's physical address space. Protected mode is described in great detail in the Intel architecture manuals. At this point you only have to understand that translation of segmented addresses (segment:offset pairs) into physical addresses happens differently in protected mode, and that after the transition offsets are 32 bits instead of 16.
    2. Second, the boot loader reads the kernel from the hard disk by direcly accessing the IDE disk device registers via the x86's special I/O instructions. You will not need to learn much about programming specific devices in this class: writing device drivers is in practice a very important part of OS development, but from a conceptual or architectural viewpoint it is also one of the least interesting.

### 1.3.1 Code: Assembly bootstrap

The first instruction in the boot loader is `cli`(boot/bootasm.S:15),  which disables processor interrupts. Interrupts are a way for hardware devices to invoke operating system functiongs called interrupt handlers. The BIOS is a tiny operating system, and it might have set up its own interrupt handlers as part of the initializing the hardware. But the BIOS isn't running anymore(the boot loader is), so it is no longer appropriate or safe to handle interrupts from hardware devices. When xv6 is ready, it will re-enable interrupts.

Now, the processor is in `real mode`,  so there are eight 16-bit general-purpose registers, but the processor sends 20 bits of adderss to memory. The segment registers %cs, %ds, %es, and %ss provide the additional bits necessary to generate 20-bit memory addresses from 16-bit registers. When a program refers to a memory address, the processor automatically adds 16 times the value of one of the segment registers; these registers are 16 bits wide. Which segment register is usually implicit in the kind of memory reference: instruction fetches use %cs, data reads and writes use %ds, and stack reads and writes use %ss.

Xv6 pretends that an x86 instruction uses a virtual address for its memory operands, but an x86 instruction actually uses a logical address. A logical address consists of a segment selector and an offset, and is sometimes written as segment:offset. More often, the segment is implicit and the program only directly manipulates the offset. The segmentation hardware performs the translation described above to generate a linear address. If the paging hardware is enabled, it translates linear addresses to physical addresses; otherwise the processor uses linear addresses as physical addresses.

The boot loader does not enable the paging hardware; the logical addresses that it uses are translated to linear addresses by the segmentation harware, and then used directly as physical addresses. Xv6 configures the segmentation hardware to translate logical to linear addresses without change, so that they are always equal. For historical reasons we have used the term virtual address to refer to addresses manipulated by programs; an xv6 virtual address is the same as an x86 logical address, and is equal to the linear address to which the segmentation hardware maps it. Once paging is enabled, the only interesting address mapping in the system will be linear to physical.

The BIOS dose not guarantee anything about the contents of %ds, %es, %ss, so first order of business after disabling interrupts is to set %ax to zero and then copy that zero into %ds, %es, and %ss(boot/bootasm.S:18-21).

A virtual *segment:offset* can yield a 21-bit physical address, but the Intel 8088 could only address 20 bits of memory, so it discarded the top bit: 0xffff0 + 0xffff = 0x10ffef, but virtual address address 0xffff:0xffff on the 8088 referred to physical address 0x0ffef. Some early software relied on the hardware ignoring the 21st address bit, so when Intel introduced processors with more than 20 bits of physical address, IBM provided a compatibility hack that is a requirement for PC-compatible hardware. If the second bit of the keyboard controller's output port is low, the 21st physical address bit is always cleared; if high, the 21st bit acts normally. The boot loader must enable the 21st address bit using I/O to the keyboard controller on ports 0x64 and 0x60(boot/bootasm.S:25-39).

Real mode's 16-bit general-purpose and segment registers make it awkward for a program to use more than 65,536 bytes of memory, and impossible to use more than a megabyte. x86 processors since the 80286 have a *protected mode*, which allows physical addresses to have many more bits, and (since the 80386) a "32-bit" mode that causes registers, virtual addresses, and most integer arithmetic to be carried out with 32 bits rather than 16. The xv6 boot sequence enables protected mode and 32-bit mode as follows.

In protected mode, a segment register is an index into a *segment descriptor table*. Each table entry specifies a base physical address, a maximum virtual address called the limit, and permission bits for the segment. These permissions are the protection in protected mode: the kernel can use them to ensure that a program uses only its own memory.

xv6 makes almost no use of segments; it uses the paging hardware instead. The boot loader sets up the segment descriptor table *gdt*(boot/bootasm.S:76-79) so that all segments have a base address of zero and the maximum posible limit(four gigabytes). The table has a null entry, one entry for executable code, and one entry to data. The code segment descriptor has a flag set that indicates that the code should run in 32-bit mode(inc/mmu.h:60). With this setup, when the boot loader enters protected mode, logical addresses map one-to-one to physical address.

The boot loader executes an `lgdt` instruction(boot/bootasm.S:44) to load the processor's global descriptor table (GDT) register with the value *gdtdesc*(boot/bootasm.S:81-83), which points to the table *gdt*.

Once it has loaded the GDT register, the boot loader enables protected mode by setting the 1 bit (CR0_PE_ON) in register %cr0(boot/bootasm.S:45-47). Enabling protected mode does not immediately change how the processor translates logical to physical address; it is only when one loads a new value into a segment register that the processor reads the GDT and changes its internal segmentation settings. One cannot directly modify %cs, so instead the code executes an `ljmp` (far jump) instruction(boot/bootasm.S:52), which allows a code segment selector to be specified. The jump continues execution at the next line(boot/bootasm.S:55) but in doing so sets %cs to refer to the code descriptor entry in *gdt*. That descriptor describes a 32-bit code segment, so the processor switches into 32-bit mode. The boot loader has nursed the processor through an evolution from 8088 through 80286 to 80386.

The boot loader's first action in 32-bit mode is to initialize the data segment registers with *PROT_MODE_DSEG*(boot/bootasm.S:57-60). Logical address now map directly to physical addresses. The only step left before executing C code is to set up a stack in an unused region of memory. The memory from 0xa0000 to 0x100000 is typically littered with device memory regions, and the xv6 kernel expects to be placed at 0x100000. The boot loader itself is at 0x7c00 through 0x7e00 (512 bytes). Essentially any onter section of memory would be a fine location for the stack. The boot loader chooses 0x7c00(known in this file as $start) as the top of the stack; the stack will grow down from there, toward 0x0000, away from the boot loader.

Finally the boot loader calls the C function bootmain(boot/bootasm.S:67). Bootmain's job is to load and run the kernel. It only returns if something has gone wrong. In that case, the code then executes an infinite loop(boot/bootasm.S:71-72). A real loader might attempt to print an error message first.

### 1.3.2 Code: C bootstrap

The C part of the boot loader, *boot/bootmain.c*, expects to find a copy of the kernel executable on the disk starting at the second sector. The kernel is an ELF format binary. To get access  to the ELF headers, *bootmain* loads the first 4096 bytes of the ELF binary(boot/bootmain.c:23). It places the in-memory copy at address 0x10000.

The next step is a quick check that this probably is an ELF binary, and not an uninitialized disk(boot/bootmain.c:26-27). *Bootmain* reads the section's content starting from the disk location off bytes after the start of the ELF header, and writes to memory starting at address paddr. *Bootmain* calls *readseg* to load data from disk(boot/bootmain.c:35).

## 1.4 The kernel

We will now start to examine the minimal xv6 kernel in a bit more detail. Like the boot loader, the kernel begins with some assembly language code that sets things up so that C language code can execute properly.

### 1.4.1 Using virtual memory to work around position dependence

When you inspected the boot loader's link and load addresses above, they matched perfectly, but there was a (rather large) disparity between the kernel's link address (as printed by objdump) and its load address. Go back and check both and make sure you can see what we're talking about. (Linking the kernel is more complicated than the boot loader, so the link and load addresses are at the top of *kern/kernel.ld*.)

Operating system kernels often like to be linked and run at very high virtual address, such as 0xf0100000, in order to leave the lower part of the processor's virtual address space for user programs to use. The reason for this arrangement will become clearer in the next lab.

Many machines don't have any physical memory at address 0xf0100000, so we can't count on being able to store the kernel there. Instead, we will use the processor's memory management hardware to map virtual address 0xf0100000 (the link address at which the kernel code expects to run) to physical address 0x00100000 (where the boot loader loaded the kernel into physical memory). This way, although the kernel's virtual address is high enough to leave plenty of address space for user processes, it will be loaded in physical memory at the 1MB point in the PC's RAM, just above the BIOS ROM.

For now, we'll just map the first 4MB of physical memory, which will be enough to get us up and running. We do this using the hand-written, statically-initialized page directory and page table in *kern/entrypgdir.c*. For now, you don't have to understand the details of how this works, just the effect that it accomplishes.

### 1.4.2 Code: Assembly entry

The first instruction in the kernel is *movw $0x1234, 0x472*(kern/entry.S:42). It writes 0x1234 to memory location 0x472. The BIOS reads this on booting to tell it to "Bypass memory test (also warm boot)".

We haven't set up virtual memory yet, so we're running from the physical address the boot loader loaded the kernel at: 1MB (plus a few bytes). However, the C code is linked to run at *KERNBASE+1MB*. Hence, we set up a trivial page directory that translates virtual addresses [KERNBASE, KERNBASE+4MB) to physical addresses [0, 4MB).  We choose 4MB because that's how much we can map with one page table and it's enough to get us through early boot. We also map virtual addresses [0, 4MB) to physical addresses [0, 4MB); this region is critical for a few instructions in entry.S and then we never use it again.

The next step is load the physical address of entry_pgdir into cr3(kern/entry.S:55-56) and turn on paging(kern/entry.S:58-60).

Now paging is enabled, but we're still running at a low EIP. Here we use *jmp* to jump above KERNBASE(kern/entry.S:65-66).

Similar to the boot loader, the only step left before executing C code is to set up a stack in an unused region of memory. We reserved space for the stack in the data section(kern/entry.S:84-88). Here we only need to set the stack pointer(kern/entry.S:70).

Finally, it calls the C function i385_init(kern/entry.S:73) which is defined in *kern/init.c*. *i386_init* doesn't return under normal circumstances. Again, we use spin as the end(kern/entry.S:76).

### 1.4.3 Exercise 1

View *kern/init.c*. The function you need to pay attention to in this file is only *i386_init*. Please complete the code.  You finally need to print "Hello, world." on the screen.  You may need to view *kern/console.h*, *kern/console.c*, *inc/stdio.h*, and so on. Here you only have to do two things:
1. Initialized the console;
2. Use *cprintf* to print "Hello, world.".

## 1.5 Exercise 2

Answer the following questions:
1. We have the following conventions in our code:
    In a function definition, the function names starts a new line. So you can `grep -n '^foo' */*.c` to find the definition of foo. It is very useful when you don't know where a function is. Find information and explain in detail. Other conventions please check in *CODING*.
2. Explain why we need to initialize the BSS.

## 1.6 References

1. Intel 64 and IA-32 Architectures Software Developer's Manual
2. [Brennan's Guide to Inline Assembly](http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html)