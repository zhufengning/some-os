#include "font/output.h"
#include "limine.h"
#include <stddef.h>
#include <stdint.h>

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
  kprint_write_info();
  // We're done, just hang...
  hcf();
}
