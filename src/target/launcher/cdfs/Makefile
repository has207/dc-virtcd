AS = sh-elf-as -little
LD = sh-elf-ld -EL
CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar

OBJS = cdfs.o syscall.o

clean :
	-rm -f virtcd.elf $(OBJS)

syscall.o : cdfs/syscall.s

cdfs.o : cdfs/cdfs.c cdfs/cdfs.h


