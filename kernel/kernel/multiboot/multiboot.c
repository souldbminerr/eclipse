#include "multiboot.h"
#include <stdio.h>

static multiboot_info_t *mb_info = NULL;

void multiboot_init(u32 magic, u32 info_addr) {
    if (magic != MULTIBOOT_MAGIC) {
        printf("[multiboot] Invalid magic: 0x%x\n", magic);
        return;
    }
    if (info_addr == 0) {
        printf("[multiboot] No multiboot info provided!\n");
        return;
    }
    u64 addr = info_addr;
    mb_info = (multiboot_info_t *)addr;
    printf("[multiboot] multiboot info initialized.\n");
}

multiboot_info_t *multiboot_get_info(void) {
    return mb_info;
}

multiboot_tag_mmap_t *multiboot_get_mmap(void) {
    if (!mb_info) return NULL;

    u64 tag_addr = (u64)mb_info + sizeof(multiboot_info_t);

    while (1) {
        multiboot_tag_t *tag = (multiboot_tag_t *)tag_addr;

        if (tag->type == MB2_TAG_END)
            break;

        if (tag->type == MB2_TAG_MMAP)
            return (multiboot_tag_mmap_t *)tag;

        // advance to next tag, aligned to 8 bytes
        tag_addr += (tag->size + 7) & ~7;
    }

    printf("[multiboot] No memory map tag found!\n");
    return NULL;
}
