#include "font/output.h"
#include <stdint.h>

static const char *EXCEPTIONS[] = {
    "Division exception",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "???",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "???",
    "x87 exception",
    "Alignment check",
    "Machine check",
    "SIMD exception",
    "Virtualisation"
};

struct __attribute__((packed)) idt_entry
{
    uint16_t isr_low;   // The lower 16 bits of the ISR's address
    uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS
                        // before calling the ISR
    uint8_t ist;        // The IST in the TSS that the CPU will load into RSP; set to
                        // zero for now
    uint8_t attributes; // Type and attributes; see the IDT page
    uint16_t isr_mid;   // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;  // The higher 32 bits of the ISR's address
    uint32_t reserved;  // Set to zero
};

struct __attribute__((packed)) idtr
{
    uint16_t limit;
    uint64_t base;
};

uint16_t get_CS()
{
    uint64_t ret = 0;
    __asm__ volatile("movq %%cs,%0\n\t" : "=r"(ret) :);
    return ret;
}

static struct idt_entry idt[256];
static struct idtr idtr;

struct interrupt_frame;
uint64_t interrupt_handlers[32];

__attribute__((no_caller_saved_registers)) void real_interrupt_handler(int vector, struct interrupt_frame *frame)
{
    kprintf("坏掉了：%d %s", vector, EXCEPTIONS[vector]);
}

__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *frame)
{
    kputs("Double fault!");
    
    //uint64_t rsp;
    //__asm__ ("mov %%rsp, %0" : "=r"(rsp));
    //kputuhex(rsp);
    __asm__ volatile("cli;hlt");
}

// 返回到发生错误的地方，不停止的话会无限触发同样的错误。
#define G_INTERRUPT_HANDLER(x)                                                                                         \
    __attribute__((interrupt)) void interrupt_handler_##x(struct interrupt_frame *frame)                               \
    {                                                                                                                  \
        real_interrupt_handler(x, frame);                                                                              \
        __asm__ volatile("cli;hlt");                                                                                 \
    }

G_INTERRUPT_HANDLER(0);
G_INTERRUPT_HANDLER(1);
G_INTERRUPT_HANDLER(2);
G_INTERRUPT_HANDLER(3);
G_INTERRUPT_HANDLER(4);
G_INTERRUPT_HANDLER(5);
G_INTERRUPT_HANDLER(6);
G_INTERRUPT_HANDLER(7);
G_INTERRUPT_HANDLER(9);
G_INTERRUPT_HANDLER(10);
G_INTERRUPT_HANDLER(11);
G_INTERRUPT_HANDLER(12);
G_INTERRUPT_HANDLER(13);
G_INTERRUPT_HANDLER(14);
G_INTERRUPT_HANDLER(15);
G_INTERRUPT_HANDLER(16);
G_INTERRUPT_HANDLER(17);
G_INTERRUPT_HANDLER(18);
G_INTERRUPT_HANDLER(19);
G_INTERRUPT_HANDLER(20);
G_INTERRUPT_HANDLER(21);
G_INTERRUPT_HANDLER(22);
G_INTERRUPT_HANDLER(23);
G_INTERRUPT_HANDLER(24);
G_INTERRUPT_HANDLER(25);
G_INTERRUPT_HANDLER(26);
G_INTERRUPT_HANDLER(27);
G_INTERRUPT_HANDLER(28);
G_INTERRUPT_HANDLER(29);
G_INTERRUPT_HANDLER(30);
G_INTERRUPT_HANDLER(31);

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags, uint64_t ist)
{
    struct idt_entry *descriptor = &idt[vector];

    descriptor->isr_low = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs = get_CS();
    descriptor->ist = ist;
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
    idtr.limit = (uint16_t)sizeof(struct idt_entry) * 32 - 1;
    kprintf("idt:%x\n", &idt[0]);
    kprintf("cs:%x\n", get_CS());
    idt_set_descriptor(0, interrupt_handler_0, 0x8E, 0);
    idt_set_descriptor(1, interrupt_handler_1, 0x8E, 0);
    idt_set_descriptor(2, interrupt_handler_2, 0x8E, 0);
    idt_set_descriptor(3, interrupt_handler_3, 0x8E, 0);
    idt_set_descriptor(4, interrupt_handler_4, 0x8E, 0);
    idt_set_descriptor(5, interrupt_handler_5, 0x8E, 0);
    idt_set_descriptor(6, interrupt_handler_6, 0x8E, 0);
    idt_set_descriptor(7, interrupt_handler_7, 0x8E, 0);
    idt_set_descriptor(8, double_fault_handler, 0x8E, 1);
    idt_set_descriptor(9, interrupt_handler_9, 0x8E, 0);
    idt_set_descriptor(10, interrupt_handler_10, 0x8E, 0);
    idt_set_descriptor(11, interrupt_handler_11, 0x8E, 0);
    idt_set_descriptor(12, interrupt_handler_12, 0x8E, 0);
    idt_set_descriptor(13, interrupt_handler_13, 0x8E, 0);
    idt_set_descriptor(14, interrupt_handler_14, 0x8E, 0);
    idt_set_descriptor(15, interrupt_handler_15, 0x8E, 0);
    idt_set_descriptor(16, interrupt_handler_16, 0x8E, 0);
    idt_set_descriptor(17, interrupt_handler_17, 0x8E, 0);
    idt_set_descriptor(18, interrupt_handler_18, 0x8E, 0);
    idt_set_descriptor(19, interrupt_handler_19, 0x8E, 0);
    idt_set_descriptor(20, interrupt_handler_20, 0x8E, 0);
    idt_set_descriptor(21, interrupt_handler_21, 0x8E, 0);
    idt_set_descriptor(22, interrupt_handler_22, 0x8E, 0);
    idt_set_descriptor(23, interrupt_handler_23, 0x8E, 0);
    idt_set_descriptor(24, interrupt_handler_24, 0x8E, 0);
    idt_set_descriptor(25, interrupt_handler_25, 0x8E, 0);
    idt_set_descriptor(26, interrupt_handler_26, 0x8E, 0);
    idt_set_descriptor(27, interrupt_handler_27, 0x8E, 0);
    idt_set_descriptor(28, interrupt_handler_28, 0x8E, 0);
    idt_set_descriptor(29, interrupt_handler_29, 0x8E, 0);
    idt_set_descriptor(30, interrupt_handler_30, 0x8E, 0);
    idt_set_descriptor(31, interrupt_handler_31, 0x8E, 0);

    __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
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
