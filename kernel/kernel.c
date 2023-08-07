#include "font/output.h"
#include "limine.h"
#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "gdt.h"
// Halt and catch fire function.
void hcf(void)
{
  __asm("cli");
  for (;;)
  {
    __asm("hlt");
  }
}

int f(int x) {
  kputint(x);
  volatile int a[1000000000]={0};
  a[99999] = 1;

  if (x == -1) return x;
  return f(x+1);
}

void _start(void)
{
  output_init();
  kputs("你好世界\n");
  // kprint_write_info();
  set_idt();
  gdt_init();
  kputhex(f(0));
  //volatile int a = *(int*)0x2398549275;
  kputs("bye\n");
  // __asm__("movl $1,%%eax"
  //         "\n\tmovl $0, %%ecx"
  //         "\n\tcltd"
  //         "\n\tidivl %%ecx" ::
  //             : "%eax", "%ecx");
  //__asm__ volatile("int $3");
  //  We're done, just hang...
  hcf();
}
