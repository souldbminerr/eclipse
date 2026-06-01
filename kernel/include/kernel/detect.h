#ifndef _KERNEL_DETECT_H
#define _KERNEL_DETECT_H

#include <stdint.h>

void detect_cpu(void);
void detect_ram(uint32_t mbi_addr);

#endif
