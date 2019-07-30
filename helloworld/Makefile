TOP := .
OBJDIR := obj
CC := gcc
OBJCOPY := objcopy
QEMU := qemu-system-i386
CFLAGS := -nostdlib -ffreestanding -fno-asynchronous-unwind-tables -fno-pic -fno-stack-protector -fno-omit-frame-pointer -m16 -O2
SRCFILES := src/helloworld.c \
			lib/serial.c
LSFILE := src/boot.ld

all: $(OBJDIR)/helloworld.bin

$(OBJDIR)/helloworld.bin: $(OBJDIR)/helloworld sign.pl
	$(OBJCOPY) -O binary $< $@
	perl sign.pl $@

$(OBJDIR)/helloworld: $(SRCFILES) $(LSFILE)
	@echo cc + $(SRCFILES)
	@mkdir -p $(@D)
	gcc -I$(TOP) $(CFLAGS) -o $@ $(SRCFILES) -T $(LSFILE)

qemu: $(OBJDIR)/helloworld.bin
	$(QEMU) -hda $<

qemu-nox: $(OBJDIR)/helloworld.bin
	@echo "***"
	@echo "*** Use Ctrl-a x to exit qemu"
	@echo "***"
	$(QEMU) -hda $< -nographic

qemu-gdb: $(OBJDIR)/helloworld.bin
	$(QEMU) -hda $< -S -s

qemu-nox-gdb: $(OBJDIR)/helloworld.bin
	@echo "***"
	@echo "*** Use Ctrl-a x to exit qemu"
	@echo "***"
	$(QEMU) -hda $< -nographic -S -s

clean:
	rm -rf $(OBJDIR)