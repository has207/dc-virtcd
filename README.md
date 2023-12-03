DC-VIRTCD
=========

DC-VIRTCD is a CD virtualization system for the Dreamcast.  It allows you
to run software which accesses files on CD-ROM, without actually burning
those files on a CD-ROM.  Instead, the files are accessed over the network.
This happens transparantly, without any modifications having to be made
to the software itself.

Requirements
------------

* Broadband adapter
* A computer to run the host side of the application on
* A network cable

Features
--------

* Works with any software using Gdc system calls to read the disc
* Supports ISO only for now

Misfeatures
-----------

* No support for CD-DA
* Does not work with e.g. NetBSD, which accesses the GD-ROM drive directly

Bulding
-------

* (host) autoconf && autoupdate && ./configure && make
* (target) make

IMPORTANT: Adjust DREAMCAST_IP and PC_IP in target/launcher/main.c before compiling.
Original version of this code attempted to use network settings in system flash on
the Dreamcast and broadcast to find the server. Both endpoints are now specified
at compile time instead.

This means the two machines no longer need to be on the same subnet, however the subnet
of the PC still needs to be routable from the Dreamcast. This means it will
not work on Windows using WSL since it creates a virtualized network (unless you put
in additional effort to make it routable somehow), and it
won't work in a virtual machine unless you use bridged networking to ensure the
VM gets an IP on the same network as the host.

Running
-------
On the PC run ./host/cmdline/dc-virtcd-cmdline /path/to/game.iso
It will wait for connections from DC so you can leave this running.

Next start ./target/launcher/virtcd.elf on the Dreamcast (using dcload-ip is fine).

If host code is running when you load the elf on the DC the download will start
and you should load into the game.
