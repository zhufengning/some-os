#include "font/output.h"
#include "limine.h"
#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
// Halt and catch fire function.
void hcf(void)
{
  asm("cli");
  for (;;)
  {
    asm("hlt");
  }
}

void _start(void)
{
  output_init();
  kputs("你好世界\n");
  // kprint_write_info();
  set_idt();
  // __asm__("movl $1,%%eax"
  //         "\n\tmovl $0, %%ecx"
  //         "\n\tcltd"
  //         "\n\tidivl %%ecx" ::
  //             : "%eax", "%ecx");
  __asm__ volatile("int $3");
  kputs("bye\n");
  //  We're done, just hang...
  hcf();
}
