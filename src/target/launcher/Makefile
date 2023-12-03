AS = sh-elf-as -little
LD = sh-elf-ld -EL
CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar
OBJCOPY = sh-elf-objcopy
CFLAGS = -Os -I$(top_srcdir)/common -I$(top_srcdir)/network

top_srcdir=..
top_builddir=..


BASEADDR=0x8cf40000

OBJS = startup.o main.o util2.o video.o launch.o handler.o cdfs.o syscall.o
LIBS = $(top_builddir)/network/libnetwork.a $(top_builddir)/common/libcommon.a

all : virtcd.elf

virtcd.elf : $(OBJS)
	$(CC) -o $@ -Wl,--oformat,elf32-shl,-Ttext=$(BASEADDR) -nostartfiles -nostdlib $(OBJS) $(LIBS) -lgcc
	kos-strip --strip-debug virtcd.elf

handler.o : $(top_builddir)/handler/sub.bin
	$(KOS_BASE)/utils/bin2o/bin2o $^ subcode $@
	kos-strip --strip-debug handler.o

clean :
	-rm -f virtcd.elf $(OBJS)

main.o : main.c video.h $(top_srcdir)/common/proto.h $(top_srcdir)/network/pci.h $(top_srcdir)/network/ether.h $(top_srcdir)/common/util.h

video.o : video.c video.h

startup.o : startup.s

launch.o : launch.s

util2.o : util2.c $(top_srcdir)/common/util.h video.h

syscall.o : syscall.s

cdfs.o : cdfs.c cdfs.h


