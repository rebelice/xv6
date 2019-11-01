# 4 lab4 Multiprocessor and Lock

`One week`

[toc]

## 4.1 Introduction

### 4.1.1 Objectives

In this lab, you will add multiprocessor support to xv6. In addition, you also need to implement the lock. You also need to complete all exercises and submit code and lab reports.

### 4.1.2 Materials

Use Git to commit your changes after Lab 2 submission (if any), fetch the latest version of the course repository, and then merge the lab4 branch:

```git
git pull
git merge origin/lab4
```

## 4.2 Multiprocessor support

We are going to make xv6 support "symmetric multiprocessing" (SMP), a multiprocessor model in which all CPUs have equivalent access to system resources such as memory and I/O buses. While all CPUs are functionally identical in SMP, during the boot process they can be classified into two types: the bootstrap processor (BSP) is responsible for initializing the system and for booting the operating system; and the application processors (APs) are activated by the BSP only after the operating system is up and running. Which processor is the BSP is determined by the hardware and the BIOS. Up to this point, all your existing xv6 code has been running on the BSP.

In an SMP system, each CPU has an accompanying local APIC (LAPIC) unit. The LAPIC units are responsible for delivering interrupts throughout the system. The LAPIC also provides its connected CPU with a unique identifier. In this lab, we make use of the following basic functionality of the LAPIC unit (in kern/lapic.c):

- Reading the LAPIC identifier (APIC ID) to tell which CPU our code is currently running on (see cpunum()).
- Sending the STARTUP interprocessor interrupt (IPI) from the BSP to the APs to bring up other CPUs (see lapic_startap()).

A processor accesses its LAPIC using memory-mapped I/O (MMIO). In MMIO, a portion of physical memory is hardwired to the registers of some I/O devices, so the same load/store instructions typically used to access memory can be used to access device registers. You've already seen one IO hole at physical address 0xA0000 (we use this to write to the VGA display buffer). The LAPIC lives in a hole starting at physical address 0xFE000000 (32MB short of 4GB),  and we have established a map here. If not, please return to lab2.

### 4.2.1 Application processor bootstrap

Before booting up APs, the BSP should first collect information about the multiprocessor system, such as the total number of CPUs, their APIC IDs and the MMIO address of the LAPIC unit. The mp_init() function in kern/mpconfig.c retrieves this information by reading the MP configuration table that resides in the BIOS's region of memory.

The boot_aps() function (in kern/init.c) drives the AP bootstrap process. APs start in real mode, much like how the bootloader started in boot/boot.S, so boot_aps() copies the AP entry code (kern/mpentry.S) to a memory location that is addressable in the real mode. Unlike with the bootloader, we have some control over where the AP will start executing code; we copy the entry code to 0x7000 (MPENTRY_PADDR), but any unused, page-aligned physical address below 640KB would work.

After that, boot_aps() activates APs one after another, by sending STARTUP IPIs to the LAPIC unit of the corresponding AP, along with an initial CS:IP address at which the AP should start running its entry code (MPENTRY_PADDR in our case). The entry code in kern/mpentry.S is quite similar to that of boot/boot.S. After some brief setup, it puts the AP into protected mode with paging enabled, and then calls the C setup routine mp_main() (also in kern/init.c). boot_aps() waits for the AP to signal a CPU_STARTED flag in cpu_status field of its struct CpuInfo before going on to wake up the next one.


### 4.2.2 Per-CPU state and initialization

When writing a multiprocessor OS, it is important to distinguish between per-CPU state that is private to each processor, and global state that the whole system shares. kern/cpu.h defines most of the per-CPU state, including struct CpuInfo, which stores per-CPU variables. cpunum() always returns the ID of the CPU that calls it, which can be used as an index into arrays like cpus. Alternatively, the macro thiscpu is shorthand for the current CPU's struct CpuInfo.

Here is the per-CPU state you should be aware of:

- Per-CPU kernel stack.
- Per-CPU TSS and TSS descriptor.
- Per-CPU registers.

## 4.3 Lock

Our current code spins after initializing the AP in mp_main(). Before letting the AP get any further, we need to first address race conditions when multiple CPUs run kernel code simultaneously. The simplest way to achieve this is to use a big kernel lock. The big kernel lock is a single global lock that is held whenever an environment enters kernel mode, and is released when the environment returns to user mode. In this model, environments in user mode can run concurrently on any available CPUs, but no more than one environment can run in kernel mode; any other environments that try to enter kernel mode are forced to wait.

## 4.4 Exercise

### 4.4.1 Exercise 1

Read boot_aps() in kern/init.c, and the assembly code in kern/mpentry.S. Make sure you understand the control flow transfer during the bootstrap of APs. Then please implement mp_main(). 

### 4.4.2 Exercise 2

The code in seg_init() (kern/vm.c) initializes the GDT. It worked in Lab 3, but is incorrect when running on other CPUs. Change the code so that it can work on all CPUs. 

### 4.4.3 Exercise 3

Please implement the `spinlock` in kern/spinlock.c

###  4.4.4 Exercise 4

Answer the following questions:

1. Compare kern/mpentry.S side by side with boot/boot.S. Bearing in mind that kern/mpentry.S is compiled and linked to run above KERNBASE just like everything else in the kernel, what is the purpose of macro MPBOOTPHYS? Why is it necessary in kern/mpentry.S but not in boot/boot.S? In other words, what could go wrong if it were omitted in kern/mpentry.S? 
2. It seems that using the big kernel lock guarantees that only one CPU can run the kernel code at a time. Why do we still need separate kernel stacks for each CPU? Describe a scenario in which using a shared kernel stack will go wrong, even with the protection of the big kernel lock.

## 4.5 Challenge

Challenge! The big kernel lock is simple and easy to use. Nevertheless, it eliminates all concurrency in kernel mode. Most modern operating systems use different locks to protect different parts of their shared state, an approach called fine-grained locking. Fine-grained locking can increase performance significantly, but is more difficult to implement and error-prone. If you are brave enough, drop the big kernel lock and embrace concurrency in xv6!

Challenge!  The spinlock is too easy. You may want to implement other complex locks, such as rwlock, mcs, mutex. Let's do it!