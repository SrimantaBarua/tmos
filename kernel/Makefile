# Makefile for the shuos kernel

ARCH?=x86_64
LD:=$(ARCH)-elf-ld

LDFLAGS:=-nostdlib --gc-sections
LIBS:=

KERNEL:=shuos.kernel

OBJS:=

include arch/$(ARCH)/make.config

OBJS+=$(ARCH_OBJS)
LDFLAGS+=$(ARCH_LDFLAGS)
LIBS+=$(ARCH_LIBS)

.PHONY: all clean test_grub

all: $(KERNEL)

clean:
	rm -f $(KERNEL) $(OBJS)

test_grub: all
	./scripts/test_grub.sh

$(KERNEL): $(OBJS)
	$(LD) -n -T $(ARCH_LDSCRIPT) -o $@ $^ $(LDFLAGS) $(LIBS)

