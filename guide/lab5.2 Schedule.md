## 5.4 Schedule

`Two week`

### 5.4.1 Introduction

Your task in this lab is to change the xv6 kernel so that it can alternate between multiple processes in "round-robin" fashion.  To achieve this goal, you need to do the following things:

#### 5.4.1.1 Exercise 4

Now you are familiar enough with the framework code.  Next, complete the following tasks and add anything you need. In addition, different ideas will have different implementations. So please think carefully about what you need to do, then start.

- Implement the `schedule()` function in kern/proc.c
  - `schedule()` is the implementation of "round-robin"
  - you can also implement other scheduling strategy
- In order to be able to run more processes, you need to implement more system calls
  - `sys_yield()`:  it lets the current process give up CPU
  - `sys_fork()`: it is able to generate new child processes
- Handling clock interrupts
  - you need to acknowledge the interrupt using lapic_eoi() before calling the scheduler when you are handling clock interrupts

### 5.4.2 Challenge!

If you have enough strength, here are some challenges you can try.

#### Inter-Process communication (IPC)

We've been focusing on the isolation aspects of the operating system, the ways it provides the illusion that each program has a machine all to itself. Another important service of an operating system is to allow programs to communicate with each other when they want to. It can be quite powerful to let programs interact with other programs. The Unix pipe model is the canonical example.

There are many models for interprocess communication. Even today there are still debates about which models are best. We won't get into that debate. Instead, we'll implement a simple IPC mechanism and then try it out.

You will implement a few additional xv6 kernel system calls that collectively provide a simple interprocess communication mechanism. You will implement two system calls, `sys_ipc_recv` and `sys_ipc_try_send`. Then you will implement two library wrappers `ipc_recv` and `ipc_send`.

The "messages" that user environments can send to each other using xv6's IPC mechanism consist of two components: a single 32-bit value, and optionally a single page mapping. Allowing environments to pass page mappings in messages provides an efficient way to transfer more data than will fit into a single 32-bit integer, and also allows environments to set up shared memory arrangements easily.

You can reference material and implement it.