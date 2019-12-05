# 6 Drivers

[TOC]

## 6.1 Introduction

`One week`

In this lab, you will write the disk driver for your operating system which is now falling out of fashion, which interface is simple enough to get started quickly.

The main job you need to do in this lab is to implement a disk drivers with IDE device in PIO mode(these concepts will be detailed below).

There are two challenge for you to choose if you like: write another driver you like, or write a disk scheduling instead of "First-Come, First-Served".

### 6.1.1 Getting started

​	git merge lab5

### 6.1.2  Lab requirements

In this lab. implement a disk drivers and answer the following questions in a few words. Place the answers in a file called answers-lab5.txt in the top level of your lab directory before handing in your work.

## 6.2 I/O Architecture

**There is a summary of the "Too Long To See" version in `6.2.4`.**

Before entering the lab, make sure you understand the I/O Architecture: I/O Subsystem --- Driver --- I/O Port --- I/O    --- I/O Controller --- Device.

In this lab, we should find out:

1. the usage of I/O Interface: In another word, the correspondence between I/O Controller and I/O Interface (One-to-many,usually two)
2. the binding relationship between I/O port and I/O Interface.

Current disk controller chips almost always support two ATA (a kind of Interface,and it will be detailed below) buses per chip, and an ATA bus typically has ten I/O ports which control its behavior (the former). There is a standardized set of IO ports to control the disks on the buses (the latter).

The first two buses are called the Primary and Secondary ATA bus, and are almost always controlled by IO ports 0x1F0 through 0x1F7, and 0x170 through 0x177, respectively (unless you change it). The associated Device Control Registers/Alternate Status ports are IO ports 0x3F6, and 0x376, respectively. The associated Drive Address Register ports are IO ports 0x3F7 and 0x377. The standard IRQ for the Primary bus is IRQ14, and IRQ15 for the Secondary bus. You can read [ATA PIO Mode](https://wiki.osdev.org/ATA_PIO_Mode) for more details.

### 6.2.1 IDE and ATA

**In a word,** IDE is the bus between Disk I/O Controller and CPU, ATA is the interface of Disk I/O Controller, but we usually use IDE to describe this interface.

IDE (Integrated Drive Electronics) refers to the electrical specification of the cables which connect ATA drives (like hard drives) to another device. **In a word,** it's also a kind of I/O Interface like UART and USB. 

ATA (Advanced Technology Attachment) refers to the technical specification itself.

Specifically, the name IDE is often used **interchangeably** with ATA, but "IDE" actually refers to only the electrical specifications of the signals on the 40 / 80 pin disk cable. ATA is the proper name for the entire specification.

### 6.2.2 PIO

PIO is one of the working modes of IDE device. The remaining two are DMA and UDMA (maybe more than these two). 

### 6.2.3 LBA

[LBA](https://wiki.osdev.org/LBA) stands for Linear Block Address. It is a way of determining the position of a block (usually on a storage medium). Most storage devices support specifying LBA's in some way or another.

CHS (Cylinder Head Sector) is an older way of addressing a storage device.

### 6.2.4 Summary

I/O ports are a serial of registers.

Each I/O interface is design by one device controller, which is several specific register of I/O ports. There might be more than one I/O interface for a specific device controller.

[IDE](https://wiki.osdev.org/PCI_IDE_Controller) and [ATA](https://wiki.osdev.org/Category:ATA) are ambiguous, but we can consider IDE as the device, ATA as the transmission protocol and technology.

ATA as a protocol, has several modes to transfer data where [PIO](https://wiki.osdev.org/ATA_PIO_Mode) is one of them. **(if you have any bug with data transfer,please first determine if you are not considering [400ns delay](https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays))**

When you transfer data from or back to disk, you should determine the position of a block. [LBA](https://wiki.osdev.org/LBA)'s are used to access certain blocks on a storage medium. In PIO modes,the LBA is one of the parameters you give to I/O port.

## 6.3 Drivers

A driver is the piece of code in an operating system that manages a particular device: it provides interrupt handlers for a device, causes a device to perform operations, causes a device to generate interrupts, etc. 

Driver code can be tricky to write because a driver executes concurrently with the device that it manages. In addition, the driver must understand the device’s interface (e.g., which I/O ports do what), and that interface can be complex and poorly documented.

In this lab, we will focus on the disk driver. The disk driver copies data from and back to the disk. Disk hardware traditionally presents the data on the disk as a numbered sequence of 512-byte blocks (also called sectors): sector 0 is the first 512 bytes, sector 1 is the next, and so on. To represent disk sectors, an operating system has a data structure that corresponds to one sector. The data stored in this data structure is often out of sync with the disk: it might have not yet been read in from disk (the disk is working on it but hasn’t returned the sector’s content yet), or it might have been updated but not yet written out. The driver must ensure that the rest of operating system doesn’t get confused when the structure is out of sync with the disk.

### 6.3.1 block device and buffer

In this lab, we need to implement a disk driver. The disk driver represent disk sectors with a data structure called a buffer,`struct buf` (buf.h). Each buffer represents the contents of one sector on a particular disk device. The dev and sector fields give the device and sector number and the data field is an in-memory copy of the disk sector.

The flags track the relationship between memory and disk: the B_VALID flag means that data has been read in, and the B_DIRTY flag means that data needs to be written out. The B_BUSY flag is a lock bit; it indicates that some process is using the buffer and other processes must not. When a buffer has the B_BUSY flag set, we say the buffer is locked.

### 6.3.2 Ideinit

#### interrupt init

The kernel initializes the disk driver at boot time by calling `ideinit` () from `main` (). `Ideinit` calls `picenable` and `ioapicenable` to enable the IDE_IRQ interrupt (). The call to `picenable` enables the interrupt on a uniprocessor; `ioapicenable` enables the interrupt on a multiprocessor, but only on the last CPU (`ncpu-1`): on a two-processor system, CPU 1 handles disk interrupts.

Next, `ideinit` probes the disk hardware. It should begin by calling `idewait` to wait for the disk to be able to accept commands. A PC motherboard presents the status bits of the disk hardware on I/O port 0x1f7. `Idewait` should polls the status bits until the busy bit `IDE_BSY` is clear and the ready bit `IDE_DRDY` is set.

#### disk check

Now that the disk controller should be ready, if everything is correct. `ideinit` should check how many disks are present. It can assume that disk 0 is present, because the boot loader and the kernel were both loaded from disk 0, but it must check for disk 1. It can write to I/O port 0x1f6 to select disk 1 and then waits a while for the status bit to show that the disk is ready. If not, `ideinit` will assume the disk is absent. 

### 6.3.3 Iderw

After `ideinit`, the disk is not used again until the buffer cache calls `iderw`, which updates a locked buffer as indicated by the flags. If B_DIRTY is set, `iderw` writes the buffer to the disk; if B_VALID is not set, `iderw` reads the buffer from the disk. Disk accesses typically take milliseconds, a long time for a processor. The boot loader issues disk read commands and reads the status bits repeatedly until the data is ready. 

> Q: why we use polling in boot loader, but use interrupt here?

`Iderw` takes this latter approach, keeping the list of pending disk requests in a queue(in challenge program, you should implement a algorithm to scheduling the queue) and using interrupts to find out when each request has finished. Although `iderw` maintains a queue of requests, the simple IDE disk controller can only handle one operation at a time. The disk driver maintains the invariant that it has sent the buffer at the front of the queue to the disk hardware; the others are simply waiting their turn. `Iderw` adds the buffer b to the end of the queue (). If the buffer is at the front of the queue, `iderw` must send it to the disk hardware by calling `idestart`; otherwise the buffer will be started once the buffers ahead of it are taken care of(time for scheduling). 

### 6.3.4 Idestart

`Idestart` issues either a read or a write for the buffer’s device and sector, according to the flags. If the operation is a write, `idestart` must supply the data now and the interrupt will signal that the data has been written to disk. If the operation is a read, the interrupt will signal that the data is ready, and the handler will read it. Note that `idestart` has detailed knowledge about the IDE device(**IMPORTANT**,you can get the details on [osdev](wiki.osdev.org/ATA_PIO_Mode)), and writes the right values at the right ports. If any of these `outb` statements is wrong, the IDE will do something differently than what we want. Getting these details right is one reason why writing device drivers is challenging.

### 6.3.5 Ideintr

Having added the request to the queue and started it if necessary, `iderw` must wait for the result. As discussed above, polling does not make efficient use of the CPU. Instead, `iderw` sleeps, waiting for the interrupt handler to record in the buffer’s flags that the operation is done. While this process is sleeping, operating system will schedule other processes to keep the CPU busy. Eventually, the disk will finish its operation and trigger an interrupt. trap will call `ideintr` to handle it. In the normal lab,`Ideintr` consults the first buffer in the queue, while in the challenge lab you should get the correct buffer in the queue, to find out which operation was happening. If the buffer was being read and the disk controller has data waiting, `ideintr` reads the data into the buffer with `insl` . Now the buffer is ready: `ideintr` sets B_VALID, clears B_DIRTY, and wakes up any process sleeping on the buffer. Finally, `ideintr` must pass the next waiting buffer to the disk.



## 6.4 Exercise

Implement buf.h

According to 6.3.1-6.3.5 and the comment, finish `ide.c`

Register the IRQ in `traps.c`



Answer the questions in this guidebook.  

> Q: in this lab and xv6, we implement the disk driver in the kernel, but in jos, it implement the disk driver as a user process(kernel only make very few adjustments to provide the necessary support). What are the advantages of each of them (there are no standard answers to this open question. **hint:** you can think from the direction of the microkernel)? What problems might you encounter when implementing a disk driver in the user process? What kind of special support does the kernel need to provide hard drive in the user part?

