--------------------------------------------------------------------------
NOTICE by David Horrocks.
davidhorrocks@btinternet.com

WARNING! The XA 6502 cross compiler is written by A.Fachat and is 
protected by the General Public Licence (GPL). The full source code and 
documentation in is original form is located in the xa.zip file which is 
inside this folder. The latest full release of XA may be found here:
http://www.floodgap.com/retrotech/xa

The files and folders inside the XA folder only are derived from XA version 
v1.2.4h and are intended to minimally allow the Hoxs64 project to be 
compiled from a single repository on an x86 platform with out the need to 
separately configure an installation of the XA compiler. It should be 
noted that the Hoxs64 executable binary incorporates output from the XA 
compiler but Hoxs64 does not embed or link to any part of the XA 
compiler.
--------------------------------------------------------------------------
This is the README file for the package 'xa', written by A.Fachat. 

The package 'xa' is provided 'as is', no warranty will be taken
for any damage caused by it or by any use of it.

The package contains the source code for the Cross assembler 'xa', several
utilities in the 'misc' subdirectory, as well as example programs and
the rest of this directory tree. As of release 2.1.4f a gcc-go32 DOS
cross-compiling make option is included.

The current version for 'xa' is 2.1.4f, as of 03mar1998

The whole package is placed under the GNU Public License, for further
information on redistribution see the included file "COPYING".
--------------------------------------------------------------------------

XA is a 6502 cross compiler:

 - under GNU public license

 - can produce _relocatable_ binaries

 - The full fileformat description and 6502 file loader included.

 - also included relocation and info utilites, as well as linker

 - for any ANSI-C compliant computer (only utilities need 'stat' call 
   for file size). 

 - fast by hashtables

 - Rockwell CMOS opcodes

 - running under DOS and any ANSI C system (Unix, Amiga, Atari ST)

I developed this cross assembler for the 6502 CPU family quite some time
ago on my Atari ST. The assembler has successfully been ported to Amiga 
and Unix computer (ported? just compiled... :-) 
Lately I came across the problem to need relocatable 6502 binary files, so
I revised the assembler from version 2.0.7 to 2.1.0, adding a (admittedly
proprietary) 6502 relocatable binary format. But there are not many other
formats around and they didn't fit my needs. I have developed this format 
myself and it is under the GNU public license.
With version 2.1.1 the 'official' version of the fileformat is supported.

To compile it, just type "make" (if you have the GNU gcc. If not, edit the 
Makefile for the compiler options). This produces "xa", the cross assembler;
"uncpk", a small packing utility (where the C64 counterpart is in the
examples subdirectory), "printcbm", that lists C64 BASIC files and
'file65' that prints some information about o65 files.  The "loader" in
the loader subdirectory is a basic 6502 implementation of a relocating
binary loader.
"file65" prints file information on 'o65' relocatable files. "reloc65"
can relocate 'o65' files.

If you want to use it under DOS, you have to have the GO32 DOS crosscompiling
tools to compile. Then just type "make dos" and you'll end up with the
appropriate DOS binaries. This has been tested only under i386 Linux, however. 
Another archive with the DOS binaries included is provided.

One problem on the Atari was it's broken "malloc". Therefore I used to
alloc everything in one chunk and divide the memory by hand. So everything
was kind of statically allocated. This is almost gone now. Only the
temporary storage between pass1 and pass2 and the preprocessor are still
allocated in one chunk (size definitions in xah.h). The rest is allocated
as needed.

The docs are in the 'doc' subdir. There also is a description of the
6502 relocatable binary format. If you think some things could be 
expressed in a better way, feel free and mail me to improve my english ;-)

For further information:

         Andre Fachat
         email: a.fachat@physik.tu-chemnitz.de


