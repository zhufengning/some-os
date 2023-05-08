#include <stdarg.h>
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
  int scale;
  // unsigned long fb_length;
};
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

struct WriteInfo write_info;
unsigned int fcolor = 0x009999ff, bcolor = 0x00000000;

void set_fcolor(unsigned int color)
{
  fcolor = color;
}

void set_bcolor(unsigned int color)
{
  bcolor = color;
}

unsigned int get_fcolor()
{
  return fcolor;
}

unsigned int get_bcolor()
{
  return bcolor;
}

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
  write_info.scale = 2;
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

void kputchar(int c)
{
  if (c != '\n')
  {
    int x = write_info.xp * (write_info.xs * write_info.scale + write_info.d);
    int y = write_info.yp * (write_info.ys * write_info.scale + write_info.d);
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
    for (int i = 0; i <= write_info.xs * write_info.scale; ++i)
    {
      for (int j = 0; j <= write_info.ys * write_info.scale; ++j)
      {
        int bnewx = (px + i / write_info.scale), bnewy = (py + j / write_info.scale + 12);
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
  if (write_info.yp * (write_info.ys * write_info.scale + write_info.d) >= write_info.yr)
  {
    write_info.yp = 0;
    write_info.xp += 1;
  }
  if (write_info.xp * (write_info.xs * write_info.scale + write_info.d) >= write_info.xr)
  {
    for (int i = 0; i < write_info.xr; ++i)
    {
      for (int j = 0; j < write_info.yr; ++j)
        if (i >= write_info.xr - write_info.xs * write_info.scale)
        {
          draw_pixel(i, j, bcolor);
        }
        else
        {
          draw_pixel(i, j, get_pixel(i + write_info.xs * write_info.scale, j));
        }
    }
    while (write_info.xp * (write_info.xs * write_info.scale + write_info.d) >= write_info.xr)
    {
      write_info.xp -= 1;
    }
  }
}

void kputs(char *p)
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
      return; // Invalid UTF-8 sequence.
    }

    for (int i = 1; i < bytes; i++)
    {
      p++;
      if ((*p & 0xC0) != 0x80)
      {
        return; // Invalid UTF-8 sequence.
      }
      codepoint = (codepoint << 6) | (*p & 0x3F);
    }

    kputchar(codepoint);
    p++;
  }
}

void kputsn(char *s, int n)
{
  char *p = s;
  while (*p && p - s + 1 <= n)
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
      return; // Invalid UTF-8 sequence.
    }

    for (int i = 1; i < bytes; i++)
    {
      p++;
      if ((*p & 0xC0) != 0x80)
      {
        return; // Invalid UTF-8 sequence.
      }
      codepoint = (codepoint << 6) | (*p & 0x3F);
    }

    kputchar(codepoint);
    p++;
  }
}

void kputuint(unsigned int x)
{
  char s[10] = {0};
  int len = 0;
  while (x)
  {
    s[len] = '0' + x % 10;
    len += 1;
    x /= 10;
  }
  if (len == 0)
  {
    kputchar('0');
  }
  else
  {
    for (int i = len - 1; i >= 0; --i)
    {
      kputchar(s[i]);
    }
  }
}

void kputuhex(unsigned int x)
{
  uint8_t s[10] = {0};
  int len = 0;
  while (x)
  {
    s[len] = x % 16;
    len += 1;
    x /= 16;
  }
  kputs("0x");
  if (len == 0)
  {
    kputchar('0');
  }
  else
  {
    for (int i = len - 1; i >= 0; --i)
    {
      if (s[i] <= 9)
      {
        kputchar('0' + s[i]);
      }
      else
      {
        kputchar('a' + s[i] - 10);
      }
    }
  }
}

void kputint(int x)
{
  if (x >= 0)
    kputuint(x);
  else
  {
    kputchar('-');
    kputuint(-x);
  }
}
void kputhex(int x)
{
  if (x >= 0)
    kputuhex(x);
  else
  {
    kputchar('-');
    kputuhex(-x);
  }
}

int strlen(char *s)
{
  int len = 0;
  while (*(s + len) != '\0')
  {
    ++len;
  }
  return len;
}
void kprintf(char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  char *st = format;
  while (*format != '\0')
  {
    if (*format == '%')
    {
      kputsn(st, format - st);
      st = format + 2;
      switch (*(format + 1))
      {
      case 'd':
        kputint(va_arg(ap, int));
        break;
      case 'u':
        kputuint(va_arg(ap, unsigned int));
        break;
      case 'x':
        kputuhex(va_arg(ap, unsigned int));
        break;
      default:
        kputchar('?');
        break;
      }
      ++format;
    }
    ++format;
  }
  kputsn(st, format - st);
  va_end(ap);
}

void kprint_write_info()
{
  kprintf("帧缓冲调试信息：\nxr:%d\nyr:%d\nxp:%d\nyp:%d\nxs:%d\nys:%d\nd:%d\npitch:%d\nfb_addr:%x\n",
          write_info.xr,
          write_info.yr,
          write_info.xp,
          write_info.yp,
          write_info.xs,
          write_info.ys,
          write_info.d,
          write_info.pitch,
          write_info.fb_addr);
}
