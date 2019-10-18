# 3 lab3 Interrupts and Exceptions

`One week`

[toc]

## 3.1 Introduction

### 3.1.1 Objectives

In this lab, you will know that xv6 how to handle interrupts and exceptions. You need to complete all exercises and submit code and lab reports.

### 3.1.2 Materials

Use Git to commit your changes after Lab 2 submission (if any), fetch the latest version of the course repository, and then merge the lab3 branch:

```git
git pull
git merge origin/lab3
```

## 3.2 Systems calls, exceptions, and interrupts

When running a process, a CPU executes the normal processor loop: read an instruction, advance the program counter, execute the instruction, repeat. But there are events on which control from a user program must transfer back to the kernel instead of executing the next instruction. These events include a device signaling that it wants attention, a user program doing something illegal (e.g., references a virtual address for which there is no page table entry), or a user program asking the kernel for a service with a system call. There is a main challenge in handling these events: the kernel must arrange that a processor switches from user mode to kernel mode.

There are three cases when control must be transferred from a user program to the kernel. First, a system call: when a user program asks for an operating system service. Second, an *exception*: when a program performs an illegal action. Examples of illegal actions include divide by zero, attempt to access memory for a page-table entry that is not present, and so on. Third, an *interrupt*: when a device generates a signal to indicate that it needs attention from the operating system. For example, a clock chip may generate an interrupt every 100 msec to allow the kernel to implement time sharing. As another example, when the disk has read a block from disk, it generates an interrupt to alert the operating system that the block is ready to be retrieved.

The kernel handles all interrupts, rather than processes handling them, because in most cases only the kernel has the required privilege and state. For example, in order to time-slice among processes in response the clock interrupts, the kernel must be involved, if only to force uncooperative processes to yield the processor.

In all three cases, the operating system design must arrange for the following to happen. The system must save the processor's registers for future transparent resume. The system must be set up for execution in the kernel. The system must chose a place for the kernel to start executing. The kernel must be able to retrieve information about the event, e.g., system call arguments. It must all be done securely; the system must maintain isolation of user processes and the kernel.

To achieve this goal the operating system must be aware of the details of how the hardware handles system calls, exceptions, and interrupts. In most processors these three events are handled by a single hardware mechanism. For example, on the x86, a program invokes a system call by generating an interrupt using the *int* instruction. Similarly, exceptions generate an interrupt too. Thus, if the operating system has a plan for interrupt handling, then the operating system can handle system calls and exceptions too.

The basic plan is follows. An interrupts stops the normal processor loop and starts executing a new sequence called an *interrupt handler*. Before starting the interrupt handler, the processor saves its registers, so that the operating system can restore them when it returns from the interrupt. A challenge in the transition to and from the interrupt handler is that the processor should switch from user mode to kernel mode, and back.

A word on terminology: Although the official x86 term is interrupt, xv6 refers to all of these as *traps*, largely because it was the term used by the PDP11/40 and therefore is the conventional Unix term. This chapter uses the terms trap and interrupt interchangeably, but it is important to remember that traps are caused by the current process running on a processor (e.g., the process makes a system call and as a result generates a trap), and interrupts are caused by devices and may not be related to the currently running process. For example, a disk may generate an interrupt when it is done retrieving a block for one process, but at the time of the interrupt some other process may be running. This property of interrupts makes thinking about interrupts more difficult than thinking about traps, because interrupts happen concurrently with other activities. Both rely, however, on the same hardware mechanism to transfer control between user and kernel mode securely, witch we will discuss next.

## 3.3 X86 protection

The x86 has 4 protection levels, numbered 0 (most privilege) to 3 (least privilege). In practice, most operating systems use only 2 levels: 0 and 3, which are then called *kernel mode* and *user mode*, respectively. The current privilege level with which the x86 executes instructions is stored in %cs register, in the field CPL.

On the x86, interrupt handlers are defined in the interrupt descriptor table (IDT). The IDT has 256 entries, each giving the %cs and %eip to be used when handling the corresponding interrupt.

To make a system call on the x86, a program invokes the *int n* instruction, where n specifies the index into the IDT. The *int* instruction performs the following steps:

- Fetch the *n*'th descriptor from the IDT, where *n* is the argument of *int*.
- Check that CPL in %cs is <= DPL, where DPL is the privilege level in the descriptor.
- Save %esp and %ss in CPU-internal registers, but only if the target segment selector's PL < CPL.
- Load %ss and %esp from a task segment descriptor.
- Push %ss.
- Push %esp.
- Push %eflags.
- Push %cs.
- Push %eip.
- Clear the IF bit in %eflags, but only on an interrupt.
- Set %cs and %eip to the values in the descriptor.

The *int* instruction is a comlex instruction, and one might wonder whether all these actions are necessary. For example, the check CPL <= DPL allows the kernel to forbid *int* calls to inappropriate IDT entries such as device interrupt routines. For a user program to execute *int*, the IDT entry's DPL must be 3. If the user program doesn't have the appropriate privilege, then *int* will result in *int* 13, which is a general protection fault. As another example, the *int* instruction cannot use the user stack to save values, because the process may not have a valid stack pointer; instead, the hardware uses the stack specified in the task segment, which is set by the kernel.

If the *int* instruction didn't require a privilege-level change, the x86 won't save %ss and %esp. After both cases, %eip is pointing to the address specified in the descriptor table, and the instruction at that address is the next instruction to be executed and the first instruction of the handler for *int n*. It is job of the operating system to implement these handlers.

An operating system can use the *iret* instruction to return from an *int* instruction. It pops the saved values during the *int* instruction from the stack, and resumes execution at the saved %eip.

## 3.4 Exercise

There are detailed comments in the code we provide, read them and complete the exercised below.

### 3.4.1 Exercise 1

First, you need to create a new segment descriptor for xv6. Now you need to add user code segment and user data segment. Please implement function seg_init() in kern/vm.c .

### 3.4.2 Exercise 2

Implement kern/trap.c and kern/trapasm.S. *vectors.pl* is used to generate *vectors.S*, run:
```shell
./vectors.pl > vectors.S
```
For more on interrupts, please see the intel 80386 Programmer's Manual.

### 3.4.3 Exercise 3

Answer the following questions:

1. Explain the process flow of xv6 after the interruption.