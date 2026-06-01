#include <stdio.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/detect.h>
#include "multiboot/multiboot.h"
#include "mmu/pmm.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_addr) {
	terminal_initialize();
	printf("Eclipse OS\n");
	multiboot_init(multiboot_magic, multiboot_info_addr);
	pmm_init();
	void *a = pmm_alloc();
    void *b = pmm_alloc();
    printf("[pmm] alloc: 0x%x 0x%x\n", (u32)a, (u32)b);
    pmm_free(a);
    void *c = pmm_alloc();
    printf("[pmm] after free: 0x%x (should match first)\n", (u32)c);

}
