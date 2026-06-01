DEFAULT_HOST=$(shell ../default-host.sh)
HOST?=$(DEFAULT_HOST)
HOSTARCH=$(shell ../target-triplet-to-arch.sh $(HOST))

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)
BUILDDIR=build

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_ARCH_OBJS_SRC=\
$(ARCHDIR)/boot.S \
$(ARCHDIR)/tty.c \

KERNEL_ARCH_OBJS=$(KERNEL_ARCH_OBJS_SRC:$(ARCHDIR)/%.c=$(BUILDDIR)/$(ARCHDIR)/%.o)
KERNEL_ARCH_OBJS:=$(KERNEL_ARCH_OBJS:$(ARCHDIR)/%.S=$(BUILDDIR)/$(ARCHDIR)/%.o)

KERNEL_C_SRCS=$(shell find . -path ./arch -prune -o -name '*.c' -print)
KERNEL_S_SRCS=$(shell find . -path ./arch -prune -o -name '*.S' -print)
KERNEL_NASM_SRCS=$(shell find . -path ./arch -prune -o -name '*.s' -print)
KERNEL_SRCS_OBJS=$(KERNEL_C_SRCS:%.c=$(BUILDDIR)/%.o)
KERNEL_SRCS_OBJS+=$(KERNEL_S_SRCS:%.S=$(BUILDDIR)/%.o)
KERNEL_SRCS_OBJS+=$(KERNEL_NASM_SRCS:%.s=$(BUILDDIR)/%.o)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
$(KERNEL_SRCS_OBJS) \

OBJS=\
$(BUILDDIR)/$(ARCHDIR)/crti.o \
$(BUILDDIR)/$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(BUILDDIR)/$(ARCHDIR)/crtend.o \
$(BUILDDIR)/$(ARCHDIR)/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(BUILDDIR)/$(ARCHDIR)/crti.o \
$(BUILDDIR)/$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(BUILDDIR)/$(ARCHDIR)/crtend.o \
$(BUILDDIR)/$(ARCHDIR)/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: eclipse.kernel

eclipse.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)

$(BUILDDIR)/$(ARCHDIR)/crtbegin.o $(BUILDDIR)/$(ARCHDIR)/crtend.o:
	mkdir -p $(dir $@)
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -MD -MF $(BUILDDIR)/$*.d -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(BUILDDIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) -MD -MF $(BUILDDIR)/$*.d -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

clean:
	rm -f eclipse.kernel
	rm -rf $(BUILDDIR)

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: eclipse.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp eclipse.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
