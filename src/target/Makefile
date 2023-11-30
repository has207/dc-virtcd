AS = sh-elf-as -little
LD = sh-elf-ld -EL
CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar
CFLAGS = -Os

all clean :
	cd common && make $@
	cd network && make $@
	cd handler && make $@
	cd launcher && make $@
	make local-$@

local-all :

local-clean :
