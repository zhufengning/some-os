#include <stdint.h>
#include "font/output.h"

struct gdt_descriptor {
    uint16_t limit;
    uint16_t base_low16;
    uint8_t  base_mid8;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high8;
};

struct tss_descriptor {
    uint16_t length;
    uint16_t base_low16;
    uint8_t  base_mid8;
    uint8_t  flags1;
    uint8_t  flags2;
    uint8_t  base_high8;
    uint32_t base_upper32;
    uint32_t reserved;
};

struct gdt {
    struct gdt_descriptor descriptors[11];
    struct tss_descriptor tss;
};

struct gdtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct tss {
    uint32_t unused0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t unused1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t unused2;
    uint32_t iopb;
} __attribute__((packed));

static struct gdt gdt;
static struct gdtr gdtr;
static struct tss tss;

#define STACK_SIZE 4096*5
static uint64_t STACK[STACK_SIZE] = {0};

void gdt_init(void);
void gdt_reload(void);
void gdt_load_tss(struct tss *tss);

void gdt_init(void) {
    kputs("Loading gdt.\n");

    // Null descriptor.
    gdt.descriptors[0].limit       = 0;
    gdt.descriptors[0].base_low16  = 0;
    gdt.descriptors[0].base_mid8   = 0;
    gdt.descriptors[0].access      = 0;
    gdt.descriptors[0].granularity = 0;
    gdt.descriptors[0].base_high8  = 0;

    // Kernel code 16.
    gdt.descriptors[1].limit       = 0xffff;
    gdt.descriptors[1].base_low16  = 0;
    gdt.descriptors[1].base_mid8   = 0;
    gdt.descriptors[1].access      = 0b10011010;
    gdt.descriptors[1].granularity = 0b00000000;
    gdt.descriptors[1].base_high8  = 0;

    // Kernel data 16.
    gdt.descriptors[2].limit       = 0xffff;
    gdt.descriptors[2].base_low16  = 0;
    gdt.descriptors[2].base_mid8   = 0;
    gdt.descriptors[2].access      = 0b10010010;
    gdt.descriptors[2].granularity = 0b00000000;
    gdt.descriptors[2].base_high8  = 0;

    // Kernel code 32.
    gdt.descriptors[3].limit       = 0xffff;
    gdt.descriptors[3].base_low16  = 0;
    gdt.descriptors[3].base_mid8   = 0;
    gdt.descriptors[3].access      = 0b10011010;
    gdt.descriptors[3].granularity = 0b11001111;
    gdt.descriptors[3].base_high8  = 0;

    // Kernel data 32.
    gdt.descriptors[4].limit       = 0xffff;
    gdt.descriptors[4].base_low16  = 0;
    gdt.descriptors[4].base_mid8   = 0;
    gdt.descriptors[4].access      = 0b10010010;
    gdt.descriptors[4].granularity = 0b11001111;
    gdt.descriptors[4].base_high8  = 0;

    // Kernel code 64.
    gdt.descriptors[5].limit       = 0;
    gdt.descriptors[5].base_low16  = 0;
    gdt.descriptors[5].base_mid8   = 0;
    gdt.descriptors[5].access      = 0b10011010;
    gdt.descriptors[5].granularity = 0b00100000;
    gdt.descriptors[5].base_high8  = 0;

    // Kernel data 64.
    gdt.descriptors[6].limit       = 0;
    gdt.descriptors[6].base_low16  = 0;
    gdt.descriptors[6].base_mid8   = 0;
    gdt.descriptors[6].access      = 0b10010010;
    gdt.descriptors[6].granularity = 0;
    gdt.descriptors[6].base_high8  = 0;

    // SYSENTER related dummy entries
    gdt.descriptors[7] = (struct gdt_descriptor){0};
    gdt.descriptors[8] = (struct gdt_descriptor){0};

    // User code 64.
    gdt.descriptors[9].limit       = 0;
    gdt.descriptors[9].base_low16  = 0;
    gdt.descriptors[9].base_mid8   = 0;
    gdt.descriptors[9].access      = 0b11111010;
    gdt.descriptors[9].granularity = 0b00100000;
    gdt.descriptors[9].base_high8  = 0;

    // User data 64.
    gdt.descriptors[10].limit       = 0;
    gdt.descriptors[10].base_low16  = 0;
    gdt.descriptors[10].base_mid8   = 0;
    gdt.descriptors[10].access      = 0b11110010;
    gdt.descriptors[10].granularity = 0;
    gdt.descriptors[10].base_high8  = 0;

    // TSS.
    gdt.tss.length       = 104;
    gdt.tss.base_low16   = 0;
    gdt.tss.base_mid8    = 0;
    gdt.tss.flags1       = 0b10001001;
    gdt.tss.flags2       = 0;
    gdt.tss.base_high8   = 0;
    gdt.tss.base_upper32 = 0;
    gdt.tss.reserved     = 0;



    // Set the pointer.
    gdtr.limit = sizeof(struct gdt) - 1;
    gdtr.base  = (uint64_t)&gdt;

    tss.ist1 = &STACK+STACK_SIZE;
    kprintf("Ist1 at %x\n", tss.ist1);

    kputs("Gdt reload.\n");
    gdt_reload();
    kputs("Tss load.\n");
    gdt_load_tss(&tss);
}

void gdt_reload(void) {
    asm volatile (
        "lgdt %0\n\t"
        "push $0x28\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x30, %%eax\n\t"
        "mov %%eax, %%ds\n\t"
        "mov %%eax, %%es\n\t"
        "mov %%eax, %%fs\n\t"
        "mov %%eax, %%gs\n\t"
        "mov %%eax, %%ss\n\t"
        :
        : "m"(gdtr)
        : "rax", "memory"
    );
}

void gdt_load_tss(struct tss *tss) {
    uintptr_t addr = (uintptr_t)tss;

    gdt.tss.base_low16   = (uint16_t)addr;
    gdt.tss.base_mid8    = (uint8_t)(addr >> 16);
    gdt.tss.flags1       = 0b10001001;
    gdt.tss.flags2       = 0;
    gdt.tss.base_high8   = (uint8_t)(addr >> 24);
    gdt.tss.base_upper32 = (uint32_t)(addr >> 32);
    gdt.tss.reserved     = 0;

    asm volatile ("ltr %0" : : "rm" ((uint16_t)0x58) : "memory");

}
