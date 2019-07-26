#include <sys/mmu.h>

.set PROT_MODE_CSEG,    0x8
.set PROT_MODE_DSEG,    0x10
.set CR0_PE_ON,         0x1

.code16
.globl start
start:
    cli

    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

seta20.1:
    inb     $0x64, %al               # Wait for not busy
    testb   $0x2, %al
    jnz     seta20.1

    movb    $0xd1, %al               # 0xd1 -> port 0x64
    outb    %al, $0x64

seta20.2:
    inb     $0x64, %al               # Wait for not busy
    testb   $0x2, %al
    jnz     seta20.2

    movb    $0xdf, %al               # 0xdf -> port 0x60
    outb    %al, $0x60

    lgdt    gdtdesc
    movl    %cr0, %eax
    orl     $CR0_PE_ON, %eax
    movl    %eax, %cr0

    ljmp    $PROT_MODE_CSEG, $start32

.code32
start32:
    movw    $PROT_MODE_DSEG, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    $0, %ax                 # Zero segments not ready for use
    movw    %ax, %fs
    movw    %ax, %gs

    # Set up the stack pointer and call into C.
    movl    $start, %esp
    call    bootmain

    # Bootmain shouldn't return.
    # If it returns, spin here.
spin:
    jmp     spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt:
  SEG_NULL				# null seg
  SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)	        # data seg

gdtdesc:
  .word   0x17                            # sizeof(gdt) - 1
  .long   gdt                             # address gdt
