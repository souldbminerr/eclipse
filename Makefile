SYSTEM_HEADER_PROJECTS=libc kernel
PROJECTS=libc kernel

export MAKE?=make
export HOST?=$(shell ./default-host.sh)
export HOSTARCH=$(shell ./target-triplet-to-arch.sh $(HOST))

export AR=$(HOST)-ar
export AS=$(HOST)-as
export CC=$(HOST)-gcc

export PREFIX=/usr
export EXEC_PREFIX=$(PREFIX)
export BOOTDIR=/boot
export LIBDIR=$(EXEC_PREFIX)/lib
export INCLUDEDIR=$(PREFIX)/include

export CFLAGS?=-O2 -g
export CPPFLAGS?=

export SYSROOT=$(CURDIR)/sysroot
export CC:=$(CC) --sysroot=$(SYSROOT)

ifeq ($(findstring -elf,$(HOST)),-elf)
export CC:=$(CC) -isystem=$(INCLUDEDIR)
endif

.PHONY: all headers build iso qemu clean

all: build

headers:
	mkdir -p "$(SYSROOT)"
	for p in $(SYSTEM_HEADER_PROJECTS); do \
		$(MAKE) -C $$p DESTDIR="$(SYSROOT)" install-headers || exit 1; \
	done

build: headers
	for p in $(PROJECTS); do \
		$(MAKE) -C $$p DESTDIR="$(SYSROOT)" install || exit 1; \
	done

iso: build
	mkdir -p isodir/boot/limine
	cp sysroot/boot/eclipse.kernel isodir/boot/eclipse.kernel
	cp limine/limine-bios.sys limine/limine-bios-cd.bin isodir/boot/limine/
	printf 'timeout: 0\n\n/Eclipse OS\n    protocol: multiboot2\n    kernel_path: boot():/boot/eclipse.kernel\n' > isodir/boot/limine/limine.conf
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		isodir -o eclipse.iso

qemu: iso
	qemu-system-$(HOSTARCH) -cdrom eclipse.iso -boot d -no-reboot -no-shutdown -d int,cpu_reset -D qemu.log

clean:
	for p in $(PROJECTS); do \
		$(MAKE) -C $$p clean || exit 1; \
	done
	rm -rf sysroot isodir eclipse.iso eclipse.kernel
