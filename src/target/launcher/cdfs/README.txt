This code is borrowed wholesale from https://mc.pp.se/dc/files/cdfs.tar.gz

We end up duplicating the syscall redirection but since the naming conventions
in the cdfs code are different it's easier to just provide a new jumptable
rather than rename everything here.

The only modification done here to the original code is expose read_sectors()
so that we can use it directly to fetch IP.BIN.

It's all self-contained in this directory so hopefully does not cause any
additional confusion.
