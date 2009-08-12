dumpkeymap - Diagnostic dump and detailed description of .keymapping files
Version 4

Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
Eric Sunshine, 1 December 2000

OVERVIEW
========
This package contains the diagnostic utility dumpkeymap, as well as highly
detailed documentation describing the internal layout of the Apple/NeXT
.keymapping file.

The dumpkeymap utility displays detailed information about each .keymapping
file mentioned on the command-line.  On Apple and NeXT platforms, if no
.keymapping files are mentioned on the command-line, then it will instead
dissect the key mapping currently in use by the WindowServer and AppKit.

Documentation includes a thorough and detailed description of the internal
layout of the .keymapping file, as well as an explanation of how to interpret
the output of dumpkeymap.

The complete set of documentation is available for perusal via dumpkeymap's
manual page (dumpkeymap.1), as well as via the command-line options described
below.

    --help
	Usage summary.
    --help-keymapping
	Detailed discussion of the internal format of a .keymapping file.
    --help-output
	Explanation of dumpkeymap's output.
    --help-files
	List of key mapping-related files and directories.
    --help-diagnostics
	Explanation of diagnostic messages.

Once the manual page is been installed, documentation can also be accessed
with the Unix `man' command:

    % man dumpkeymap


COMPILATION
===========
MacOS/X, Darwin

    cc -Wall -o dumpkeymap dumpkeymap.c -framework IOKit

MacOS/X DP4 (Developer Preview 4)

    cc -Wall -o dumpkeymap dumpkeymap.c -FKernel -framework IOKit

MacOS/X Server, OpenStep, NextStep

    cc -Wall -o dumpkeymap dumpkeymap.c

By default, dumpkeymap is configured to interface with the HID driver (Apple)
or event-status driver (NeXT), thus allowing it to dump the key mapping which
is currently in use by the WindowServer and AppKit.  However, these facilities
are specific to Apple/NeXT.  In order to build dumpkeymap for non-Apple/NeXT
platforms, you must define the DUMPKEYMAP_FILE_ONLY flag when compiling the
program.  This flag inhibits use of the HID and event-status drivers and
configures dumpkeymap to work strictly with raw key mapping files.

For example, to compile for Linux:

    gcc -Wall -DDUMPKEYMAP_FILE_ONLY -o dumpkeymap dumpkeymap.c


INSTALLATION
============
Install the dumpkeymap executable image in a location mentioned in the PATH
environment variable.  Typicall locations for executable files are:

    /usr/local/bin
    $(HOME)/bin

Install the manual page, dumpkeymap.1, in the `man1' subdirectory one of the
standard manual page locations or in any other location mentioned by the
MANPATH environment variable.

Typical locations for manual pages on most Unix platforms are:

    /usr/local/man/man1

Typical locations for manual pages on MacOS/X, Darwin, and MacOS/X Server are:

    /usr/local/man/man1
    /Local/Documentation/ManPages/man1
    /Network/Documentation/ManPages/man1

Typical locations for manual pages on OpenStep and NextStep are:

    /usr/local/man/man1
    /LocalLibrary/Documentation/ManPages/man1
    /LocalDeveloper/Documentation/ManPages/man1


CONCLUSION
==========
This program and its accompanying documentation were written by Eric Sunshine
and are copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>.

The implementation of dumpkeymap is based upon information gathered on
September 3, 1997 by Eric Sunshine <sunshine@sunshineco.com> and Paul S.
McCarthy <zarnuk@zarnuk.com> during an effort to reverse engineer the format
of the NeXT .keymapping file.



$XFree86: xc/programs/Xserver/hw/darwin/utils/README.txt,v 1.2 2000/12/05 21:18:34 dawes Exp $
