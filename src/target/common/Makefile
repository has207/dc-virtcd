AS = sh-elf-as -little
LD = sh-elf-ld -EL
CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar
CFLAGS = -Os

OBJS = proto.o util1.o

LIBTARGET = libcommon.a

all : $(LIBTARGET)

clean :
	-rm -f $(LIBTARGET) $(OBJS)

$(LIBTARGET) : $(OBJS)
	$(AR) rc $@ $(OBJS)

proto.o : proto.c proto.h util.h

util1.o : util1.c util.h 

