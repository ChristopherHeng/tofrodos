# makefile.vs
# Makefile for use with Visual Studio to generate Windows binaries
# Copyright 2005-2026 by Christopher Heng. All rights reserved.

# To make todos.exe and fromdos.exe
#	nmake -f makefile.vs all

# To remove all the generated files, including the binary distribution zip
#	nmake -f makefile.vs clobber
# Note that the clobber target removes all tofrodos-*.zip files, since it doesn't know
# if the VERSION macro is correctly set to the same value as it was when the "dist" target was built.

# To make the binary distribution zip and accompanying SHA512 checksum file
#	nmake -f makefile.vs VERSION=1.9.0 dist
# Note that to make the dist target, you will also need:
#	pandoc - from https://github.com/jgm/pandoc
#	hash - from https://github.com/ChristopherHeng/hash
#	tar - the bsdtar version that is part of Windows (not GNU tar!)


# The following is used for generating the binary distribution zip; override on the nmake command line.
!ifndef VERSION
VERSION = 2.1.0
!endif

# Programs
CC = cl
CP = copy
LD = link
RM = del
# other required programs for the dist target: tar, hash, pandoc; see info at the beginning of this makefile

# Flags
CFLAGS = $(CFL1) $(CFL2)
	# /nologo	- suppress startup banner
	# /Zi		- debug - create program database
	# /W4		- warning level 4
	# /WX		- treat warnings as errors
	# /02		- maximize speed (/Og /Oi /Ot /Oy /Gs /GF /Gy)
	#		note that
	#		/Og global optimizations (implied by /O2)
	#		/Oi - enable intrinsic functions
	#		/Ot - optimize speed
	#		/Oy -
	#		/Gs stack checking calls (for local variables)
	#		/GF enable string pooling
	#		/Gy function-level linking
	# /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS"
	# /MT		- multi-threaded library
	# /utf-8 - set source and execution character sets to UTF-8
	# /Fo		- output dir
	# /I		- include directory
	# /c		- compile only
CFL1 =  /nologo /W4 /WX /O2 /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /MT /utf-8 /std:c17
CFL2 = /I"$(LIBDIR)\\" /I"$(MAKEDIR)\\" /Fo"$(BINDIR)\\" /c
LDFLAGS = $(LDFL1) $(LDFL2) $(LDFL3)
LDFL1 = /OUT:"$(FROMDOS)" /INCREMENTAL:NO /NOLOGO
LDFL2 = /SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /WX
LDFL3 = setargv.obj

# Directories
BINDIR = $(MAKEDIR)
LIBDIR = $(MAKEDIR)\lib
SYSDIR = $(MAKEDIR)\windows

# Files
FROMDOS = fromdos.exe
TODOS = todos.exe
TOFRODOS_BINARY_DIST = tofrodos-$(VERSION)-windows-x86-64.zip
TOFRODOS_BINARY_DIST_CHECKSUM = $(TOFRODOS_BINARY_DIST).sha512

OBJS =	check_and_save_file_info.obj \
	convert_file.obj \
	copy_file.obj \
	emsg.obj \
	init.obj \
	getopt.obj \
	make_filenames.obj \
	mkstemp.obj \
	process_file.obj \
	resolve_filename_and_convert.obj \
	strip_path_prefix.obj \
	tofrodos.obj \
	utility.obj

# Implicit rules
.c{$(BINDIR)}.obj:
	$(CC) @<<
  $(CFLAGS) $<
<<

# Rules
all: $(FROMDOS) $(TODOS)

clean:
	$(RM) $(OBJS) set_test_filetime.obj

clobber: clean
	$(RM) $(FROMDOS) $(TODOS) tofrodos-*.zip tofrodos-*.sha512 set_test_filetime.exe readme.html fromdos.1 tofrodos.html

test: $(FROMDOS) $(TODOS) set_test_filetime.exe
	cd tests
	runtests all

dist: fromdos.1 tofrodos.html $(TOFRODOS_BINARY_DIST) $(TOFRODOS_BINARY_DIST_CHECKSUM)

$(FROMDOS): $(OBJS)
	$(LD) @<<
  $(LDFLAGS) $(OBJS)
<<

$(TODOS): $(FROMDOS)
	$(CP) %s $@

set_test_filetime.exe: tests\tools\set_test_filetime.c
	cl /nologo /W4 /WX /O2 /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /MT /utf-8 /std:c17 %s

$(TOFRODOS_BINARY_DIST): $(TODOS) $(FROMDOS) readme.html tofrodos.html COPYING-GPL2.txt COPYING-GPL3.txt
	tar -a -c -f $@ $**

$(TOFRODOS_BINARY_DIST_CHECKSUM): $(TOFRODOS_BINARY_DIST) $(TODOS) $(FROMDOS) readme.html tofrodos.html COPYING-GPL2.txt COPYING-GPL3.txt
	hash -g -o -f $@ $**

# library
getopt.obj: $(LIBDIR)\getopt.c $(LIBDIR)\getopt.h
	$(CC) $(CFLAGS) %s

mkstemp.obj: $(LIBDIR)\mkstemp.c
	$(CC) $(CFLAGS) %s

# main objs
check_and_save_file_info.obj: $(SYSDIR)\check_and_save_file_info.c config.h emsg.h tofrodos.h
	$(CC) $(CFLAGS) %s

convert_file.obj: convert_file.c config.h emsg.h tofrodos.h

copy_file.obj: copy_file.c copy_file.h

emsg.obj: emsg.c config.h emsg.h tofrodos.h

fromdos.1: man\manual-source.md man\man-metadata.txt
	pandoc -s -t man -o $@ --metadata-file=man\man-metadata.txt %s

init.obj: init.c config.h emsg.h tofrodos.h utility.h version.h $(LIBDIR)\getopt.h

make_filenames.obj: make_filenames.c config.h emsg.h tofrodos.h utility.h

process_file.obj: process_file.c config.h emsg.h tofrodos.h utility.h

resolve_filename_and_convert.obj: $(SYSDIR)\resolve_filename_and_convert.c config.h emsg.h tofrodos.h utility.h
	$(CC) $(CFLAGS) %s

strip_path_prefix.obj: strip_path_prefix.c config.h utility.h

tofrodos.html: man\manual-source.md man\html-metadata.txt
	pandoc -s -t html -o $@ --metadata-file=man\html-metadata.txt -V mainfont=Arial -V fontsize=16pt -V maxwidth=45em %s

tofrodos.obj: tofrodos.c config.h emsg.h tofrodos.h utility.h version.h $(LIBDIR)\getopt.h

utility.obj: utility.c config.h emsg.h tofrodos.h utility.h

readme.html: README.md
	pandoc -f gfm -t html -o $@ -s -V mainfont=Arial -V fontsize=16pt %s
