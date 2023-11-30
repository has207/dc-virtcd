AS = sh-elf-as -little
LD = sh-elf-ld -EL
CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar
OBJCOPY = sh-elf-objcopy
CFLAGS = -Os -I$(top_srcdir)/common -I$(top_srcdir)/network

top_srcdir=..
top_builddir=..
srcdir=.


SUBBASEADDR=0x8c005000

OBJS = sub.o handler.o
LIBS = $(top_builddir)/network/libnetwork.a $(top_builddir)/common/libcommon.a

all : sub.bin

sub.bin : sub.elf
	$(OBJCOPY) -O binary -R .stack $^ $@

sub.elf : $(OBJS) $(LIBS)
	$(CC) -o $@ -Wl,--oformat,elf32-shl,-Ttext=$(SUBBASEADDR) -nostartfiles -nostdlib $(OBJS) $(LIBS) -lgcc

clean :
	-rm -f sub.bin $(OBJS)

sub.o : sub.s

handler.o : handler.c $(top_srcdir)/network/pci.h $(top_srcdir)/network/ether.h $(top_srcdir)/common/proto.h $(top_srcdir)/common/util.h

