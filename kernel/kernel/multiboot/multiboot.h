#pragma once
#include <stdint.h>
#include <kernel/types.h>

#define MULTIBOOT_MAGIC          0x36D76289
#define MULTIBOOT_MEMORY_AVAILABLE 1

#define MB2_TAG_END  0
#define MB2_TAG_MMAP 6

typedef struct {
    u32 total_size;
    u32 reserved;
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    u32 type;
    u32 size;
} __attribute__((packed)) multiboot_tag_t;

typedef struct {
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
    // entries follow
} __attribute__((packed)) multiboot_tag_mmap_t;

typedef struct {
    u64 addr;
    u64 len;
    u32 type;
    u32 reserved;
} __attribute__((packed)) multiboot_mmap_entry_t;

void multiboot_init(u32 magic, u32 info_addr);
multiboot_info_t *multiboot_get_info(void);
multiboot_tag_mmap_t *multiboot_get_mmap(void);
