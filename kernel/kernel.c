#include "font/output.h"
#include "limine.h"
#include <stddef.h>
#include <stdint.h>

// Halt and catch fire function.
static void hcf(void)
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
  kputchar('a');
  kputchar('b');
  kputchar('c');
  kputchar(12398);
  kputs("\n你好世界\n");
  kputuint(12345);
  kputchar('\n');
  kputuhex(12345);
  kputchar('\n');
  kprint_write_info();
  for (int i = 0; i < 100; ++i)
  {
    kputchar('a' + i % 26);
  }
  // We're done, just hang...
  hcf();
}
