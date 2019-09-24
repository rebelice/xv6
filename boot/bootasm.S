#include <inc/mmu.h>

# Start the first CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,    0x8
.set PROT_MODE_DSEG,    0x10
.set CR0_PE_ON,         0x1

.code16								# Assemble for 16-bit mode
.globl start
start:
	cli								# BIOS enabled interrupts; disable

	# Zero data segment registers DS, ES, and SS.
	xorw    %ax, %ax
	movw    %ax, %ds				# -> Data Segment
	movw    %ax, %es				# -> Extra Segment
	movw    %ax, %ss				# -> Stack Segment

	# Physical address line A20 is tied to zero so that the first PCs
	# with 2 MB would run software that assumend 1 MB. Undo that.
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

	# Switch from real to protected mode. Use a bootstrap GDT that makes
	# virtual addresses map directly to physical addresses so that the
	# effective memory map doesn't change during the transition.
	lgdt    gdtdesc
	movl    %cr0, %eax
	orl     $CR0_PE_ON, %eax
	movl    %eax, %cr0

	# Comlete the transition to 32-bit protected mode by using a long jmp
	# to reload %cs and %eip. The segment descriptors are set up with no
	# translation, so that the mapping is still the identity mapping.
	ljmp    $PROT_MODE_CSEG, $start32

.code32		# Tell assembler to generate 32-bit code now.
start32:
	# Set up the protected-mode data segment registers
	movw    $PROT_MODE_DSEG, %ax	# Our data segment selector
	movw    %ax, %ds				# -> Data Segment
	movw    %ax, %es				# -> Extra Segment
	movw    %ax, %ss				# -> Stack Segment
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
.p2align 2								# force 4 byte alignment
gdt:
  SEG_NULLASM							# null seg
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG_ASM(STA_W, 0x0, 0xffffffff)		# data seg

gdtdesc:
  .word   (gdtdesc - gdt - 1)			# sizeof(gdt) - 1
  .long   gdt							# address gdt
