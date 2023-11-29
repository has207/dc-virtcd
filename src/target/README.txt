DC side of the code is broken up as follows:

* launcher/

This is the code that runs initially and sets up syscall handler,
downloads the binaries and launches into the game.

We can reasonably do fancy things here and load additional libraries
as this code is no longer needed once the game is loaded so we are free
to use biggish memory chunk at this stage, as long as it leaves enough
room to load 1ST_READ.BIN.


* skel/

This is where we generate the code the replaces gdrom* syscalls and
contains all necessary subroutines for networking to continue working
after our initial executable is replaced by the game.

At a high level, schandler.c (subcode handler) defines the C routines
that do the communication with the server that hosts the game
and sub.s contains the assembler needed for initial handler installation
and jump table for gdrom* routines.

The code is compiled into an elf executable, which is then stripped down
to bare machine code using objcopy, which ends up stored in sub.bin.
The elf is compiled such that its text area starts at a specific memory
location. This location is hardocoded in skel/Makefile and separately
in launcher/main.c (as SUBSTART). These values can be changed to place
this code in a different location as long as they are kept in sync.

For the purpose of the launcher we wrap sub.bin as an opaque object file
that simply provides the address of where this code begins and ends.
It is referenced with "subcode" and "subcode_end" symbols in the main
executable and is simply copied into its intended destination before
being jumped into in order to do intialization.

* common/
* network/

These both contain code shared by the launcher and syscall handler,
therefore will affect the size of the patch binary.
