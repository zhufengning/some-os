#include "../limine.h"
#include <stddef.h>
#include "font.h"
struct WriteInfo
{
  int xr, yr; // 分辨率
  int xp, yp; // 坐标
  int xs, ys; // 字体大小
  int d;      // 间隔
  uint32_t *fb_addr;
  int pitch;
  // unsigned long fb_length;
};
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

struct WriteInfo write_info;
unsigned const int fcolor = 0x009999ff, bcolor = 0x00000000;

int output_init()
{
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1)
  {
    return -1;
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];
  write_info.xr = framebuffer->height;
  write_info.yr = framebuffer->width;
  write_info.xp = 0;
  write_info.yp = 0;
  write_info.xs = 16;
  write_info.ys = 16;
  write_info.d = 0;
  write_info.fb_addr = framebuffer->address;
  write_info.pitch = framebuffer->pitch;
  return 0;
}
void draw_pixel(int x, int y, unsigned int color)
{
  *(write_info.fb_addr + x * (write_info.pitch / 4) + y) = color;
}
unsigned int get_pixel(int x, int y)
{
  return *(write_info.fb_addr + x * (write_info.pitch / 4) + y);
}
void putchar(int c)
{
  if (c != '\n')
  {
    int x = write_info.xp * (write_info.xs + write_info.d);
    int y = write_info.yp * (write_info.ys + write_info.d);
    uint32_t *start = write_info.fb_addr;
    int p = 0;
    for (int i = 0; i < 22235; ++i)
    {
      if (FONT_CODE[i] == c)
      {
        p = i;
      }
    }
    int px = p / 18 * 16;
    int py = p % 18 * 27;
    for (int i = 0; i <= write_info.xs; ++i)
    {
      for (int j = 0; j <= write_info.ys; ++j)
      {
        int bnewx = px + i, bnewy = py + j + 12;
        int fnewx = x + i;
        int fnewy = y + j;
        if (FONT[bnewx * 486 + bnewy] == 1)
        {
          draw_pixel(fnewx, fnewy, fcolor);
        }
        else
        {
          draw_pixel(fnewx, fnewy, bcolor);
        }
      }
    }
    write_info.yp += 1;
  }
  else
  {
    write_info.xp += 1;
    write_info.yp = 0;
  }
  if (write_info.yp * (write_info.ys + write_info.d) >= write_info.yr)
  {
    write_info.yp = 0;
    write_info.xp += 1;
  }
  if (write_info.xp * (write_info.xs + write_info.d) >= write_info.xr)
  {
    for (int i = 0; i < write_info.xr; ++i)
    {
      for (int j = 0; j < write_info.yr; ++j)
        if (i >= write_info.xr - write_info.xs)
        {
          draw_pixel(i, j, bcolor);
        }
        else
        {
          draw_pixel(i, j, get_pixel(i + write_info.xs, j));
        }
    }
    while (write_info.xp * (write_info.xs + write_info.d) >= write_info.xr)
    {
      write_info.xp -= 1;
    }
  }
}

void puts(char *p)
{
  while (*p)
  {
    int codepoint = 0;
    int bytes;

    if ((*p & 0x80) == 0)
    {
      codepoint = *p;
      bytes = 1;
    }
    else if ((*p & 0xE0) == 0xC0)
    {
      codepoint = *p & 0x1F;
      bytes = 2;
    }
    else if ((*p & 0xF0) == 0xE0)
    {
      codepoint = *p & 0x0F;
      bytes = 3;
    }
    else if ((*p & 0xF8) == 0xF0)
    {
      codepoint = *p & 0x07;
      bytes = 4;
    }
    else
    {
      return -1; // Invalid UTF-8 sequence.
    }

    for (int i = 1; i < bytes; i++)
    {
      p++;
      if ((*p & 0xC0) != 0x80)
      {
        return -1; // Invalid UTF-8 sequence.
      }
      codepoint = (codepoint << 6) | (*p & 0x3F);
    }

    putchar(codepoint);
    p++;
  }
}