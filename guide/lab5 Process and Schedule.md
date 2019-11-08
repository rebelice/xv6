# 5 lab5 Process

`Two week`

[toc]

## 5.1 Introduction

### 5.1.1 Objectives

In this lab you will implement the basic kernel facilities required to get a protected user-mode process running. You will enhance the xv6 kernel to set up the data structures to keep track of processes, create a single user process, load a program image into it, and start it running. You will also make the xv6 kernel capable of handling any system calls the user process makes and handling any other exceptions it causes.

The purpose of the entire lab is to make the user program user/hello.c run successfully and output "hello, world", and finally return to the kernel.

### 5.1.2 Materials

Use Git to commit your changes ater Lab 4 submission (if any), fetch the latest version of the coures repository, and then merge the lab5 branch:

```git
git pull
git merge origin/lab5
```

## 5.2 Process

The new include file kern/proc.h contains basic definitions for process in xv6. You need to read it. The kernel uses the `proc` data structures to keep track of each process. In this lab you will initially create just one process, but you will need to design the xv6 kernel to support multiple processes.

Once xv6 gets up and running, the `ptable`(kern/proc.c) data structures tracks all the processes in the system. In our design, the xv6 kernel will support a maximum of `NPROC` simultaneously active process, although there will typically be far fewer running processes at any given time.

### 5.2.1 Context switching

In order to know how to create a process and start it running, you need to understand the context switching.

Every xv6 process has its own kernel stack and register set, as we saw in `proc` data structure. Each CPU has a separate scheduler thread for use when it is executing the scheduler rather than any process's kernel thread. Switching from one thread to another involves saving  the old thread's CPU registers, and restoring the previously-saved registers of the new thread; the fact the %esp and %eip are saved and restored means that the CPU will switch stacks and switch what code it is executing.

![Context switching](_v_images/20191108121342218_990441578.png)

`swtch` doesn't directly know about threads; it just saves and restores register sets, called `context`. When it is time for a process to give up the CPU, the process's kernel thread calls `swtch` to save its own context and return to the scheduler context. Each context is represented by a `struct context*`, a pointer to a structure stored on the kernel stack involved. `Swtch` takes two arguments: `struct context **old` and `struct context *new`. It pushes the current CPU register onto the stack and saves the stack pointer in `*old`. The `swtch` copies `new` to %esp, pops previously saved registers, and returns.

### 5.2.2 Creating and running processes

You will now write the code in kern/proc.c necessary to run a user process. Because we do not yet have a filesystem, we will set up the kernel to load a static binary image that is *embedded within the kernel itself*. Xv6 embeds this binary in the kernel as a ELF executable image.

The Lab 5 GNUmakefile generates a number of binary images in the obj/user/ directory. If you look at kern/Makefrag, you will notice some magic that "links" these binaries directly into the kernel executable as if they were .o files. The -b binary option on the linker command line causes these files to be linked in as "raw" uninterpreted binary files rather than as regular .o files produced by the compiler. (As far as the linker is concerned, these files do not have to be ELF images at all - they could be anything, such as text files or pictures!) If you look at obj/kern/kernel.sym after building the kernel, you will notice that the linker has "magically" produced a number of funny symbols with obscure names like _binary_obj_user_hello_start, _binary_obj_user_hello_end, and _binary_obj_user_hello_size. The linker generates these symbol names by mangling the file names of the binary files; the symbols provide the regular kernel code with a way to reference the embedded binary files.

In i386_init() in kern/init.c you'll see code to run one of these binary images in a process. However, the critical functions to set up user process are not complete; you will need to fill them in.

### 5.2.3 Exercise 1

In the file kern/proc.c, finish coding the following functions:

- proc_init()
    - Initialize the ptable.lock
- user_init()
    - Set up a single process.
- proc_alloc()
    - Find an UNUSED proc in ptable. If found, change state to EMBRYO and initialize state required to run in the kernel.
- ucode_load()
    - You will need to parse an ELF binary image, much like the boot loader already dose, and load its contents into the user address space of a new process.
- ucode_run()
    - Start  the first process running in user mode.

You also need to implement the following functions in kern/vm.c:

- region_alloc()
    - Allocates and maps physical memory for a process
- uvm_switch()
    - load the TSS and switch the page table

You need to finish coding:

    - swtch.S

### 5.2.3 System calls

Now that your kernel has basic exception handling capabilities, you will refine it to provide important operating system primitives that depend on exception handling.

User processes ask the kernel to do things for them by invoking system calls. When the user process invokes a system call, the processor enters kernel mode, the processor and the kernel cooperate to save the user process's state, the kernel executes appropriate code in order to carry out the system call, and then resumes the user process. The exact details of how the user process gets the kernel's attention and how it specifies which call it wants to execute vary from system to system.

In the xv6 kernel, we will use the int instruction, which causes a processor interrupt. In particular, we will use int $0x40 as the system call interrupt. We have defined the constant T_SYSCALL to 64 (0x40) for you. You will have to set up the interrupt descriptor to allow user processes to cause that interrupt. Note that interrupt 0x40 cannot be generated by hardware, so there is no ambiguity caused by allowing user code to generate it.

The application will pass the system call number and the system call arguments in registers. This way, the kernel won't need to grub around in the user environment's stack or instruction stream. The system call number will go in %eax, and the arguments (up to five of them) will go in %edx, %ecx, %ebx, %edi, and %esi, respectively. The kernel passes the return value back in %eax. The assembly code to invoke a system call has been written for you, in syscall() in lib/syscall.c. You should read through it and make sure you understand what is going on.

### 5.2.4 Exercise 2

Add a handler in the kernel for interrupt vector `T_SYSCALL`. You will have to edit kern/trap.c and implement the syscall in kern/syscall.c. Other than this, you need to implement the sys_cputs() and sys_exit() (call exit() in kern/proc.c) in kern/syscall.c.

Finish coding the following functions:

- kern/proc.c
    - sched()
        - context switch from thisproc to scheduler
    - forkret()
        - return to trapret
    - exit()
- kern/syscall.c
    - sys_cputs()
    - sys_call()
- kern/trap.c
    - trap()

## 5.3 Exercise 3

Answer the following questions:

1. what dose pushcli() and popcli() do? And why?