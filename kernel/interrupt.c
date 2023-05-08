#include <stdint.h>
#include <font/output.h>
struct __attribute__((__packed__)) GdtEntryBits
{
    unsigned int limit_low : 16;
    unsigned int base_low : 24;
    unsigned int accessed : 1;
    unsigned int read_write : 1;             // readable for code, writable for data
    unsigned int conforming_expand_down : 1; // conforming for code, expand down for data
    unsigned int code : 1;                   // 1 for code, 0 for data
    unsigned int code_data_segment : 1;      // should be 1 for everything but TSS and LDT
    unsigned int DPL : 2;                    // privilege level
    unsigned int present : 1;
    unsigned int limit_high : 4;
    unsigned int available : 1; // only used in software; has no effect on hardware
    unsigned int long_mode : 1;
    unsigned int big : 1;  // 32-bit opcodes for code, uint32_t stack for data
    unsigned int gran : 1; // 1 to use 4k page addressing, 0 for byte addressing
    unsigned int base_high : 8;
};

struct __attribute__((packed)) IdtEntry
{
    uint16_t isr_low;   // The lower 16 bits of the ISR's address
    uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t ist;        // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t attributes; // Type and attributes; see the IDT page
    uint16_t isr_mid;   // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;  // The higher 32 bits of the ISR's address
    uint32_t reserved;  // Set to zero
};

void set_gdt()
{
    asm("cli");
}

uint16_t get_CS()
{
    uint64_t ret = 0;
    __asm__ volatile(
        "movq %%cs,%0\n\t"
        : "=r"(ret)
        :);
    return ret;
}

struct __attribute__((packed)) IdtR
{
    uint16_t limit;
    uint64_t base;
};

static struct IdtEntry idt[256];
static struct IdtR idtr;

struct interrupt_frame;

__attribute__((interrupt)) void interrupt_handler(struct interrupt_frame *frame)
{
    kputs("坏掉了！\n");
    __asm__ volatile("cli;hlt");
}

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags)
{
    struct IdtEntry *descriptor = &idt[vector];

    descriptor->isr_low = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs = get_CS();
    descriptor->ist = 0;
    descriptor->attributes = flags;
    descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}
extern void *isr_stub_table[];
void hcf();

void set_idt()
{
    kputs("Loading idt\n");
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(struct IdtEntry) * 32 - 1;
    kprintf("idt:%x\n", &idt[0]);
    kprintf("cs:%x\n", get_CS());
    // hcf();
    for (uint8_t vector = 0; vector < 32; vector++)
    {
        idt_set_descriptor(vector, interrupt_handler, 0x8E);
        // vectors[vector] = true;
    }
    // hcf();
    __asm__ volatile("lidt %0"
                     :
                     : "m"(idtr)); // load the new IDT
    // hcf();
    __asm__ volatile("sti"); // set the interrupt flag
    kputs("Loaded\n");
}

// __attribute__((noreturn)) void exception_handler(void);
// void exception_handler()
// {
//     kputs("坏掉了\n");
//     __asm__ volatile("cli; hlt"); // Completely hangs the computer
// }