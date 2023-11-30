#include <stdarg.h>
#include <stdint.h>

#include "video.h"
#include "util.h"
#include "util2.h"

/* Dummy stub to make libgcc happy... */
void atexit() { }

/* Expect to find root directory within this many sectors from the start */
#define ROOT_DIRECTORY_HORIZON 500

struct isofs_s {
  unsigned char *data;
  uint32_t root_lba, root_length;
};

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

int strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

char to_upper(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 'A';
    }
    return ch;
}

static int ntohlp(const unsigned char *ptr)
{
  return (ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3];
}

static int fncompare(const char *fn1, int fn1len, const char *fn2, int fn2len)
{
  while(fn2len--)
    if(!fn1len--)
      return *fn2 == ';';
    else if(to_upper((unsigned char)*fn1++) != to_upper((unsigned char)*fn2++))
      return 0;
  return fn1len == 0;
}

static int isofs_find_root_directory(isofs iso)
{
  uint32_t sec;
  unsigned char *buf;
  for (sec = 16; sec < ROOT_DIRECTORY_HORIZON; sec++) {
    buf = iso->data+sec;
    if (!memcmp(buf, "\001CD001", 6))
      break;
    else if(!memcmp(buf, "\377CD001", 6))
      return 0;
  }
  if (sec >= ROOT_DIRECTORY_HORIZON) {
    printf("Unabled to find root sector\n");
    return 0;
  }
  printf("Found PVD at LBA %d", sec);
  iso->root_lba = ntohlp(buf+156+6)+150;
  iso->root_length = ntohlp(buf+156+14);
  printf("Root directory is at LBA %d, length %d bytes",
	       iso->root_lba, iso->root_length);
  return 1;
}

static int isofs_find_entry(isofs iso, const char *entryname,
			     uint32_t *sector, uint32_t *length, int enlen,
			     uint32_t dirsec, uint32_t dirlen, int dirflag)
{
  uint32_t len;
  unsigned char *buf;
  const unsigned char *p;
  dirflag = (dirflag? 2 : 0);
  while(dirlen > 0) {
    buf = iso->data + dirsec;
    if (dirlen > 2048) {
      len = 2048;
      dirlen -= 2048;
      dirsec++;
    } else {
      len = dirlen;
      dirlen = 0;
    }
    for (p=buf; len>0; ) {
      if(!*p || *p>len)
	break;
      if (*p>32 && *p>32+p[32])
	if ((p[25]&2) == dirflag &&
	    fncompare(entryname, enlen, (const char *)(p+33), p[32])) {
	  if (sector)
	    *sector = ntohlp(p+6)+150;
	  if (length)
	    *length = ntohlp(p+14);
	  return 1;
	}
      len -= *p;
      p += *p;
    }
  }
  return 0;
}

int isofs_find_file(isofs iso, const char *filename,
		     uint32_t *sector, uint32_t *length)
{
  return isofs_find_entry(iso, filename, sector, length, strlen(filename),
			  iso->root_lba, iso->root_length, 0);
}

int isofs_init(isofs iso, unsigned char *data)
{
  if (iso) {
    iso->data = data;
    if (isofs_find_root_directory(iso)) {
      return 1;
    } else
      printf("No PVD found\n");
  } else
    printf("Invalid isofs\n");
  return 0;
}
