# zssh2 (ZMODEM SSH 2)

zssh2 is a program for interactively transferring files from or to a remote
machine while using SSH. It is especially useful when sftp is not an option.

zssh2 is forked from http://zssh.sourceforge.net/.

## Installation

The readline library is required.

```sh
autoreconf
./configure
make
sudo make install
```

## Usage

See `zssh2(1)`.

Files are transferred through the ZMODEM protocol, using the rz and sz
commands. If you don't already have them, the sz/rz programs can be
installed from Uwe Ohse's lrzsz package, for more information, see
https://www.ohse.de/uwe/software/lrzsz.html

## Supported Operating Systems

- Linux
- FreeBSD
- OpenBSD
- macOS

illumos support depends on https://www.illumos.org/issues/5386.
