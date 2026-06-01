# Multiboot2 header — recognized by GRUB (multiboot2) and Limine
.set MB2_MAGIC,    0xE85250D6
.set MB2_ARCH,     0
.set MB2_HDRLEN,   mb2_end - mb2_start
.set MB2_CHECKSUM, -(MB2_MAGIC + MB2_ARCH + MB2_HDRLEN)

.section .multiboot.data, "aw"
.align 8
mb2_start:
.long MB2_MAGIC
.long MB2_ARCH
.long MB2_HDRLEN
.long MB2_CHECKSUM
# End tag (required)
.short 0
.short 0
.long  8
mb2_end:

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384
stack_top:

# Four page tables: PML4T, PDPT, PDT, PT — each 4096 bytes
.section .bss, "aw", @nobits
.align 4096
pml4t:
.skip 4096
pdpt:
.skip 4096
pdt:
.skip 4096
pt:
.skip 4096

# 64-bit GDT
.section .data
.align 8
gdt64:
    .quad 0                                           # null
gdt64_code:
    .quad (1<<44)|(1<<47)|(1<<41)|(1<<43)|(1<<53)    # code: P|S|R|E|L
gdt64_data:
    .quad (1<<44)|(1<<47)|(1<<41)                     # data: P|S|W
gdt64_end:

gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .long gdt64          # 32-bit base is fine; kernel is < 4GB

# Saved multiboot values
mb_magic: .long 0
mb_info:  .long 0

# ──────────────────────────────────────────────
# Entry point: GRUB drops us here in 32-bit protected mode
# eax = multiboot magic, ebx = multiboot info ptr
# ──────────────────────────────────────────────
.section .multiboot.text, "a"
.code32
.global _start
.type _start, @function
_start:
    mov %eax, mb_magic
    mov %ebx, mb_info

    mov $stack_top, %esp

    # ── Check CPUID ──────────────────────────────
    pushfl
    pop %eax
    mov %eax, %ecx
    xor $(1<<21), %eax
    push %eax
    popfl
    pushfl
    pop %eax
    push %ecx
    popfl
    xor %ecx, %eax
    jz no_long_mode

    # ── Check long mode support ───────────────────
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $(1<<29), %edx
    jz no_long_mode

    # ── Disable any existing paging ──────────────
    mov %cr0, %eax
    and $~(1<<31), %eax
    mov %eax, %cr0

    # ── Set up page tables ────────────────────────
    # PML4T[0] → PDPT (present | writable)
    mov $pdpt, %eax
    or $0x3, %eax
    movl %eax, pml4t

    # PDPT[0] → PDT
    mov $pdt, %eax
    or $0x3, %eax
    movl %eax, pdpt

    # PDT[0] → PT
    mov $pt, %eax
    or $0x3, %eax
    movl %eax, pdt

    # PT: identity map first 2MB (512 × 4KB pages)
    mov $pt, %edi
    mov $0x003, %ebx
    mov $512, %ecx
.fill_pt:
    movl %ebx, (%edi)
    movl $0, 4(%edi)
    add $4096, %ebx
    add $8, %edi
    loop .fill_pt

    # ── Load CR3 ─────────────────────────────────
    mov $pml4t, %eax
    mov %eax, %cr3

    # ── Enable PAE (CR4.PAE) ──────────────────────
    mov %cr4, %eax
    or $(1<<5), %eax
    mov %eax, %cr4

    # ── Set EFER.LM ──────────────────────────────
    mov $0xC0000080, %ecx
    rdmsr
    or $(1<<8), %eax
    wrmsr

    # ── Enable paging + protected mode → compatibility mode
    mov %cr0, %eax
    or $((1<<31)|(1<<0)), %eax
    mov %eax, %cr0

    # ── Load GDT and jump to 64-bit ───────────────
    lgdt gdt64_ptr
    ljmp $8, $longmode64

no_long_mode:
    cli
    hlt

# ──────────────────────────────────────────────
# 64-bit long mode
# ──────────────────────────────────────────────
.section .text
.code64
longmode64:
    mov $16, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov $stack_top, %rsp
    xor %rbp, %rbp

    # Pass multiboot values to kernel_main(uint32_t magic, uint32_t info)
    # SysV AMD64 ABI: first arg = rdi, second = rsi
    movl mb_magic(%rip), %edi
    movl mb_info(%rip), %esi

    call kernel_main

    cli
1:
    hlt
    jmp 1b
