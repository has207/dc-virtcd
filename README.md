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
* Supports ISO and NRG images

Misfeatures
-----------

* No support for CD-DA
* Does not work with e.g. NetBSD, which accesses the GD-ROM drive directly

Bulding
-------

(separately in host/ and target/)
autoconf && autoupdate && ./configure && make


Running
-------

First start ./target/launcher/virtcd.elf on the Dreamcast (dcload-ip is fine).
Quickly thereafter, on PC run ./host/cmdline/dc-virtcd-cmdline /path/to/game.iso

The binary on the Dreamcast will broadcast looking for the PC for a short while
but will time out if you wait too long.

The two machines must be on the same subnet as the communication is established
using broadcast packets that won't cross subnet boundaries. This means it will
not work on Windows using WSL since it creates a virtualized network, and it
won't work in a virtual machine unless you use bridged networking to ensure the
VM gets an IP on the same network as the host.

The Dreamcast will need a network configuration saved to flash as virtcd does
not support DHCP. XDP is probably the easiest method unless you own the
Broadband passport since there are easy to find .cdi images for XDP.

