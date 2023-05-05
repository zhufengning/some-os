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
  // Ensure we got a framebuffer.

  output_init();
  putchar('a');
  putchar('b');
  putchar('c');
  putchar(12398);
  puts("\n你好世界\n");
  // We're done, just hang...
  hcf();
}
