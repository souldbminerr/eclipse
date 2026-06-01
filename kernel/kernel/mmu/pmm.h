#pragma once
#include <kernel/types.h>

void pmm_init(void);
void* pmm_alloc(void);
void pmm_free(void* addr);
u64 pmm_free_frames(void);