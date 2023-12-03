#include <stdarg.h>
#include <stdint.h>

#include "video.h"

/* Dummy stub to make libgcc happy... */
void atexit() { }

/* Expect to find root directory within this many sectors from the start */
#define ROOT_DIRECTORY_HORIZON 500

static int do_plot_string(int x, int y, char *p, unsigned short col)
{
  int n = 0;
  while(*p) {
    video_plot_char(x, y, *(unsigned char *)p, col);
    x += 12;
    n += 12;
    p++;
  }
  return n;
}

static char *format_num(int n, int base, int w, int pad)
{
  static char buf[32];
  char *p=buf+32;
  int s=0;
  *--p = '\0';
  if(base==10 && n<0) {
    n = -n;
    s = 1;
    --w;
  }
  do {
    if(base == 16) {
      *--p = "0123456789abcdef"[n & 15];
      n = ((unsigned int)n)>>4;
    } else {
      *--p = (n % base) + '0';
      n /= base;
    }
    --w;
  } while(n);
  if(pad<0)
    pad = ' ';
  while(w>0) {
    *--p = pad;
    --w;
  }
  if(s)
    *--p = '-';
  return p;
}

void _vprintf(char *fmt, va_list va)
{
  static int x=8, y=8;
  int w=-1, pad=-1;
  unsigned short col = C_BLACK;
  while(*fmt++)
    if(fmt[-1]=='%')
      while(1) {
	switch(*fmt++) {
	 case '%':
	   video_plot_char(x, y, '%', col); x+=12;
	   break;
	 case '\0':
	   return;
	 case 's':
	   x += do_plot_string(x, y, va_arg(va, char *), col);
	   w = pad = -1;
	   break;
	 case 'c':
	   {
	     int c = va_arg(va, int);
	     video_plot_char(x, y, c, col);
	     x+=(c>0xff? 24:12);
	     w = pad = -1;
	   }
	   break;
	 case '0':
	   if(w<0) {
	     pad='0';
	     continue;
	   }
	 case '1':
	 case '2':
	 case '3':
	 case '4':
	 case '5':
	 case '6':
	 case '7':
	 case '8':
	 case '9':
	   if(w<0) w=0;
	   w = w*10 + (fmt[-1]&0xf);
	   continue;
	 case 'p':
	   w = 8;
	   pad = '0';
	 case 'd':
	 case 'x':
	   x += do_plot_string(x, y, format_num(va_arg(va, int),
						(fmt[-1]=='d'? 10:16),
						w, pad), col);
	   w = pad = -1;
	}
	break;
      }
    else if(fmt[-1]=='\n') {
      x = 8;
      y += 24;
    } else if(fmt[-1]=='\r')
      x = 8;
    else if(fmt[-1]=='\014') {
      x = y = 8;
    } else {
      video_plot_char(x, y, ((unsigned char *)fmt)[-1], col);
      x += 12;
    }
}

void printf(char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  _vprintf(fmt, va);
  va_end(va);
}

