#include "pmm.h"
#include "../multiboot/multiboot.h"
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 4096
#define MAX_FRAMES (1024 * 1024) // 4GB for now

static u32 bitmap[MAX_FRAMES / 32]; // Used to track used/free frames
static u64 total_frames = 0;
static u64 used_frames = 0;

static void frame_set(u64 frame) {
    bitmap[frame / 32] |= (1 << (frame % 32));
}

static void frame_clear(u64 frame) {
    bitmap[frame / 32] &= ~(1 << (frame % 32));
}

static int frame_test(u64 frame) {
    return bitmap[frame / 32] & (1u << (frame % 32));
}

extern char _kernel_end; // Provided by linker

void pmm_init(void) {
    multiboot_tag_mmap_t* mmap = multiboot_get_mmap();
    if(!mmap) {
        printf("[pmm] Cannot find memory map!\n");
        return;
    }


    // Mark all frames as used initially
    for (u64 i = 0; i < MAX_FRAMES / 32; i++)
        bitmap[i] = 0xFFFFFFFF;

    u64 kernel_end = (u64)&_kernel_end;

    u64 entry_addr = (u64)mmap + sizeof(multiboot_tag_mmap_t);
    u64 entry_end  = (u64)mmap + mmap->size;

    while(entry_addr < entry_end) {
        multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*)entry_addr;

        if(entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            u64 base = (entry->addr + PAGE_SIZE - 1) & ~(u64)(PAGE_SIZE - 1);
            u64 end  = (entry->addr + entry->len)    & ~(u64)(PAGE_SIZE - 1);
            for(u64 addr = base; addr < end; addr += PAGE_SIZE) {
                u64 frame = addr / PAGE_SIZE;
                if(frame < MAX_FRAMES) {
                    frame_clear(frame);
                    total_frames++;
                }
            }
        }

        entry_addr += mmap->entry_size;
    }
    // protect frame 0
    frame_set(0);
    used_frames++;

    // protect kernel
    for(u64 addr = 0; addr < kernel_end; addr += PAGE_SIZE) {
        u64 frame = addr / PAGE_SIZE;
        if(frame < MAX_FRAMES && !frame_test(frame)) {
            frame_set(frame);
            used_frames++;
        }
    }

    printf("[pmm] %u MB total, %u frames used\n",
           (u32)(total_frames * PAGE_SIZE / (1024 * 1024)),
           (u32)used_frames);
}

void* pmm_alloc(void) {
    for(u64 i = 1; i < total_frames; i++) {
        if(!frame_test(i)) {
            frame_set(i);
            used_frames++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    printf("[pmm] Out of memory!\n");
    return NULL; // Out of memory
}

void pmm_free(void* addr) {
    u64 frame = (u64)addr / PAGE_SIZE;
    if(frame == 0 || frame >= total_frames) {
        printf("[pmm] Invalid free!\n");
        return;
    }
    if(frame_test(frame)) {
        frame_clear(frame);
        used_frames--;
    }
}

u64 pmm_free_frames(void) {
    return total_frames - used_frames;
}