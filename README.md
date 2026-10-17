# Tofrodos

## Contents

- [What is Tofrodos?](#intro)
- [How to Use Tofrodos](#howtouse)
- [How to Install Tofrodos](#install)
	- [Linux, macOS and Unix-like Systems](#instunix)
	- [Windows](#instwin)
	- [MSDOS, FreeDOS and Clones](#instdos)
- [Compiling Tofrodos for Linux, macOS and Unix-like Systems](#bldunix)
- [Compiling Tofrodos for Windows](#bldwin)
- [Compiling Tofrodos for MSDOS](#blddos)
- [Porting to Other Systems](#porting)
- [History of Changes](#history)
- [Copyright and Licence](#copyright)

## <a id="intro"></a>What is Tofrodos?

DOS (and Windows) text files traditionally have CR/LF (carriage return/line feed)
pairs as their new line delimiters while Unix text files traditionally have
LFs (line feeds) to terminate each line.

Tofrodos comprises two programs, "fromdos" and "todos", which convert
ASCII and Unicode UTF-8 text files to and from these formats. Use "fromdos"
to convert DOS text files to the Unix format, and "todos" to convert Unix
text files to the DOS format.

## <a id="howtouse"></a>How to Use Tofrodos

Tofrodos is a command line program. This means that if you
use Windows, you will need to open Command Prompt.
If you use any of the Unix-like systems, such as Linux, macOS,
FreeBSD, etc, you will need to start a shell (for example, bash).

To convert a plain text file with CR-LF line endings (which
is used by Windows and DOS) to LF endings (used by
Unix-like systems), do:

	fromdos file-to-convert.txt

To convert a plain text file with LF to CR-LF, do:

	todos file-to-convert.txt

If you have more than one file to convert, you can list them
all on the command line at once, like so:

	fromdos file1.txt file2.txt file3.txt

or even use wildcard characters, such as

	todos *.txt

You can also put the program in a pipe (that is, cause it to
get its input from another program and send its output to a third
program), like so:

	cat some-file.txt | todos | diff - reference.txt

If you have files scattered across multiple subdirectories,
you can make Tofrodos recursively descend through those
directories to convert them. Take for example the following
command line:

	fromdos -r -i cpp -i h -i txt --exclude-dir=testscripts src

The `-r`, short for `--recursive`, instructs Tofrodos to check
the `src` directory and all its descendants except any
subdirectory called `testscripts`, and convert all files ending
with the `.cpp`, `.h` and `.txt` extensions. Extensions
must be specified one at a time
(no wildcards allowed) with `-i` (or `--include-extension` if
you prefer the long form). You can exclude specific directories
with `-X` (with a capital "X") or its long version `--exclude-dir`,
and specific files with `-x` (with a small "x") or its long
form `--exclude-file`. Multiple `-x` and `-X` options may be used,
and wildcards are not allowed in them.

The names *todos* and *fromdos* (or "todos.exe" and "fromdos.exe"
on Windows) are traditionally the names under which this program is
installed. But you can also rename the program files to *unix2dos*
and *dos2unix* (or "unix2dos.exe" and "dos2unix.exe" on Windows),
and they will work just fine. Under the hood, these are all the
same program file. Tofrodos checks the name under which it is run
(whether "todos", "fromdos", "unix2dos" or "dos2unix") and converts
your files accordingly.

That's it. For more advanced options, please see the
tofrodos manual. If you use Windows, open `tofrodos.html`
in your web browser. If you use Linux and have installed
Tofrodos in the usual way, type `man fromdos` or
`man todos`. You can also see a short summary of all options
by typing `todos --help` or `fromdos --help` on the command line.

## <a id="install"></a>How to Install Tofrodos

### <a id="instunix"></a>Linux, macOS and Unix-like Systems

If you use Linux, you may be able to get tofrodos from your distribution's
repository. For example, on Ubuntu, install it with

	sudo apt install tofrodos

Not all package repositories have the latest version of Tofrodos.
For example, at the time I write this, Debian and all downstream distributions
that depend on it include a very outdated version. If you want the
latest version, one solution is to compile it yourself. See the section
[Compiling Tofrodos for Linux, macOS and Unix-like Systems](#bldunix)
for more information.

### <a id="instwin"></a>Windows

Download the binary distribution files from the [tofrodos
repository's releases page](https://www.github.com/ChristopherHeng/tofrodos/releases).
It contains the precompiled 64-bit executables (binaries) for the
Windows console (ie, Command Prompt). Just unpack the contents of
the zip file into a directory of your choice.

To read the manual, open tofrodos.html (included in the
distribution file) in a web browser.

Note that if you prefer the names "dos2unix.exe" and
"unix2dos.exe", just make a copy of "todos.exe" (or "fromdos.exe")
under those names. Tofrodos automatically detects the name under
which it is run, and will change its behaviour accordingly.
(That is, "dos2unix.exe" will convert files from the DOS format to
the Unix format, and "unix2dos.exe" will convert files from the
Unix format to the DOS format.)

If you wish to compile the program from the sources yourself,
either download the sources from the releases page or clone the
repository. See the section [Compiling Tofrodos for Windows](#bldwin)
for more information.

### <a id="instdos"></a>MSDOS, FreeDOS and Clones

You will need to compile the sources. See the section
[Compiling Tofrodos for MSDOS](#blddos).

## <a id="bldunix"></a>Compiling Tofrodos for Linux, macOS and Unix-like Systems

First, get the source code from the Tofrodos repository at  
https://www.github.com/ChristopherHeng/tofrodos
That is, either download the source archive from the
[releases page](https://www.github.com/ChristopherHeng/tofrodos/releases)
or clone the repository.

Note that the source zip and tar.gz files that
GitHub automatically generates contain source files with CR/LF line
endings (since that's what my files have). As far
as I can tell, this is not a problem for gcc.
If it bothers you, clone the repository (instead of
downloading the source archives) and let Git check out your working
copy. It should then check out the files that are not managed
by `.gitattributes` using your system's native EOL (ie, with LF on
Unix-like systems).

To compile everything, just type:

	make -f makefile.gcc all

You can then install the binaries to `/usr/bin` and the man pages
to `/usr/man/man1` by typing:

	make -f makefile.gcc install

Since you're installing the binary and manual page into your
system directories with this command, you'll need to be
logged in as root. (I know this is stating the obvious,
but it's just in case you're new to installing programs.)

To install them somewhere other than the defaults,
redefine BINDIR and MANDIR for the binary and manual page
destinations respectively. For example, to install the binaries
in `/usr/local/bin` and the manual page in `/usr/local/man/man1`,
do the following:

	make -f makefile.gcc BINDIR=/usr/local/bin MANDIR=/usr/local/man/man1 install

To read the manual page after installation, simply type

	man fromdos

If you have set MANDIR to a non-standard location, so
that the above does not work, specify the path to the page
when invoking "man". For example, if the manual is in the
current directory, type:

	man ./fromdos.1

Note that if you prefer the names "dos2unix" and
"unix2dos", just rename the binaries accordingly and you're done.
Tofrodos automatically detects the name under which it is run,
and will change its behaviour accordingly. That is, "dos2unix"
will convert files from the DOS format to the Unix format,
and "unix2dos" will convert them in the other direction.
No code changes are necessary.

## <a id="bldwin"></a>Compiling Tofrodos for Windows

You do not actually need to compile Tofrodos for Windows if you
don't want to, since you can download the precompiled 64 bit binaries from
[Tofrodos' releases page](https://www.github.com/ChristopherHeng/tofrodos/releases)

For those who prefer to compile it yourself, either download the
source zip from the abovementioned releases page, or clone the
repository, which is located at:  
https://www.github.com/ChristopherHeng/tofrodos

You will need either Visual Studio or MinGW-w64's gcc.

To compile with Visual Studio, do the following:

	nmake -f makefile.vs all

To compile with the gcc that comes with MinGW-w64, do this instead:

	make -f makefile.min all

(Depending on which MinGW-w64 distribution you have installed, you may need
to invoke it with "mingw32-make" instead of "make".)

The distribution binaries were compiled with Visual Studio.

## <a id="blddos"></a>Compiling Tofrodos for MSDOS

To cross-compile for MSDOS, you will need OpenWatcom C/C++
and compile from Windows, rather than MSDOS, since many of the
files have longer filenames than MSDOS can handle.

First, get the source zip from
[Tofrodos' releases page](https://www.github.com/ChristopherHeng/tofrodos/releases)
or clone the repository, which is located at:  
https://www.github.com/ChristopherHeng/tofrodos

Next, copy the files `src\startup\wildargv.c` and `src\startup\initarg.h`
from your WATCOM directory to Tofrodos' `lib` subdirectory. For example, if
you installed Watcom to c:\watcom, and you expanded the tofrodos
package into d:\tofrodos, do the following:

	copy c:\watcom\src\startup\wildargv.c d:\tofrodos\lib
	copy c:\watcom\src\startup\initarg.h d:\tofrodos\lib

The above files are needed so that Tofrodos can process wildcards on the
command line.

Then, compile the source using the following command:

	wmake -f makefile.wcc TARGET=dos16 all

Alternatively, you can also create a 32-bit version that uses a
DOS extender:

	wmake -f makefile.wcc TARGET=dos32 all

Make sure you do a

	wmake -f makefile.wcc clean

before compiling again for a different target or the response files
and object files will be wrong for the new target.

## <a id="porting"></a>Porting to Other Systems

If you want to compile Tofrodos for a system other than those listed
above, you may or may not have some work in store for you. The
program is very small, so the tweaks you need to make are hopefully minimal.

The first place to look into is probably the `config.h` file. You will
probably also have to adapt one of the makefiles for your platform.

## <a id="history"></a>History of Changes

The dates given are when the code base was finalised and do not
necessarily refer to the date of public release.

Version 3.0.0 - 17 October 2026
- [All systems] Tofrodos can now recursively convert files in
one or more directories.
- [Unix-like systems] fixed bug where Tofrodos did not
handle a chain of symlinks (where the target of a symlink is
yet another symlink) correctly.

Version 2.1.1 20 Apr 2026
- [Unix-like systems] fixed bug where Tofrodos was unable to
convert a file in a directory that is not writeable by the user,
even though the file itself is writeable.

Version 2.1.0 29 Mar 2026
- [All systems except MSDOS] fixed bug where Tofrodos would
incorrectly handle files with multiple hard links.
- [Unix-like systems] fixed bug where in certain situations,
earlier versions of Tofrodos could fail to correctly determine a
file's read/write permissions. It only happens if you try
to convert a file that you do not own.
- [All systems] if the --preserve option is used, and Tofrodos
is not able to restore the file's original owner and/or time, it will
now issue error messages to that effect. It would previously silently
fail the change.
- [Unix-like systems] Tofrodos will no longer allow you to convert
a read-only file that you are not the owner of, even if you use
the --force option. (You can, of course, manually chmod the file to
remove the read-only flag if you have the necessary privileges.)
- In view of the non-trivial bug fixes here, this is a
recommended upgrade.

Version 2.0.0 18 March 2026
- [All systems] the long options --unix and --dos have been
renamed --unix2dos and --dos2unix respectively to make it easier to
remember what it does. You can still type --unix or --dos if you like,
since you are allowed to truncate long options provided the letters
uniquely identify the option you want. Note: you do not need to use
these options if you are using the default names for the program
(todos, fromdos, unix2dos or dos2unix), since the direction of
conversion is implicit from the name.
- [All systems] added long options --todos and --fromdos, which are
synonyms for --unix2dos and --dos2unix respectively, for those who
are more familiar with those names.
- [All systems] now checks to make sure the program is named either
"todos", "fromdos", "dos2unix" or "unix2dos" or one of
the --todos, --fromdos, --unix2dos, --dos2unix, -u, or -d options
is specified, otherwise it will not proceed with the conversion.
Previously, if the direction of conversion was not clear from the
name of the program or from its options, it would assume the
conversion was in a particular direction depending on the operating system.
This meant that the program behaved differently on different systems
if it was given some arbitrary name (other than one of the above).
- [All systems] the documentation has been updated (ie, the manual pages
and this README file). The man page and its HTML equivalent are now
generated from the same source file, making it easier to update in
the future.
- [Windows] now supports the Windows' implementation of symbolic links
introduced in Vista in 2007. Note that symlinks have been supported on
Unix-like systems (eg, Linux, macOS, *BSD, etc) since 2003 (ver 1.7).
- [Windows] now supports '/' as directory separators (in addition to
the usual '\\') for pathnames since Windows can also handle these.
(Specifically, Windows accepts both separators at the Win32 API
level, though not all Windows programs, including Microsoft's own,
support it. Now, Tofrodos also supports it.)

Version 1.9.0 10 March 2026
- [All systems] added support for long options (for example, instead
of saying typing -h, you can now type --help, etc).
- [All systems] updated Tofrodos' URL to its new home, namely its git
repository on GitHub.
- [Windows] the MinGW-w64 makefile has been updated. Apparently, newer
versions of MinGW-w64 do not enable wildcard processing by default, so
the revised makefile includes an additional module to make sure this
is turned on.

Version 1.8.4 8 Jan 2026
- [Windows/MSDOS] fixed bug in my implementation of getopt() due
to a misunderstanding of its return value when a missing option argument
is encountered. This arose from an incompatibility between the POSIX
specification and the BSD's implementation as described in its manual
page. We now align with POSIX rather than BSD for this return value.
Note that this only affects tofrodos if you compile with Visual Studio
or OpenWatcom.
- [All systems except Windows/MSDOS] fixed cryptic error message
about an invalid option when what is meant was that a required option
argument was missing (it only occurs when the user used the -l option
without supplying a filename).
- [Linux] the Linux Software Map (LSM) has been removed. This was an
artifact from the 1990s when tofrodos was first written, but I don't
think anyone uses it any more.
- [All systems] filelist.txt has been removed. It is unnecessary,
and I doubt that anyone even looked at it.

Version 1.8.3 14 June 2024
- [All systems] The -a option, long deprecated since 2013, has
been removed.
- [Windows/MSDOS] Options can now only be preceded by a '-' character,
to align it with all the other systems (eg, Linux, macOS, etc).
(You could previously also use '/'.)
- [Windows] Updated makefiles for MinGW-w64 and Visual Studio
- [Windows] Updated the precompiled binaries to 64-bit versions.
If you are still using a 32-bit version of Windows, compile the
program yourself using (say) MinGW-w64 with a 32-bit compiler.

Versions 1.8.0 to 1.8.2
- (Internal versions - not publicly released.)

Version 1.7.13	25 October 2013
- [Hurd, NetBSD, FreeBSD kernel] Added support for Hurd,
NetBSD and FreeBSD kernel (a system that uses the FreeBSD
kernel, but is not necessarily the full FreeBSD system).
As a side benefit, tofrodos is slightly more portable since
it no longer depends on certain system-specific macros
(namely MAXPATHLEN from sys/param.h).
- [All] Tofrodos now displays information on what to do if
it is not able to rename the temporary file back to the
original filename after a successful conversion.
- [All] The -a option is now documented as "deprecated",
since you shouldn't use it unless you have an unusual
text file that you're trying to fix.
- [All] Minor improvements to the documentation.

Version 1.7.12	1 October 2012
- [All] Under certain error conditions, Tofrodos may fail to
remove the temporary files that it creates. This is now fixed.
- [All] Fixed another bug where an exit code of 0, instead of 1,
is returned under certain failure conditions.
- [MSDOS] Although MSDOS is not a multitasking system, and
thus should not need it, the DOS port of Tofrodos now also uses
my implementation of mkstemp(). This simplifies maintenance
since I have fewer code paths to test.
- [Windows] Support for compiling the source code using
DJGPP 2, Borland C/C++, LCC-Win32 and Digital Mars C has been
removed. Please use one of the other supported compilers (eg,
Open Watcom C, etc).
- [All] The documentation now has information about the exit
codes returned by Tofrodos.

Version 1.7.11	27 September 2012
- [All] tofrodos now consistently returns an exit code of 1 when
there's a failure. Previously, under certain error conditions,
it could return an exit code of -1 (which is not a valid
exit code for some operating systems).
- [Windows] This version now includes an implementation of
mkstemp() for Windows (when compiled with Open Watcom C),
bringing the Windows (Open Watcom) port of tofrodos up to
par with versions for systems like Linux and Mac OS X which
provide mkstemp() in their C libraries. Hopefully, this will
help those of you who run multiple instances of tofrodos at
the same time in the same directory.
- [Windows] Fixed a bug introduced in 1.7.9 where the Windows port
of tofrodos always creates the temporary file in the current
directory instead of the directory where the target file is.
- [Mac OS X] The Mac OS X port of tofrodos now uses mkstemp() to
create the temporary file the way it does on Linux.
- [Mac OS X] Added more information in readme.txt for Mac users.
- [All] Improved documentation about the -p and -b options.

Version 1.7.10	25 September 2012
- Limited (beta) distribution only, not released for general use.
If you have this version, please upgrade to 1.7.11.

Version 1.7.9	21 February 2011
- [Windows, Linux, Mac OS X, Unix] The behaviour of the -b option
(create backup file) has been changed to be more useful on systems
with support for long filenames and an arbitrary number of file
extensions (ie, all systems except MSDOS). It now backs up
"filename.ext" as "filename.ext.bak" instead of "filename.bak".
Note that with this version, the DOS version no longer behaves
the same way (when the -b option is used) as the Windows, Linux,
Mac OS X, *BSD and other Unix versions, since DOS systems have
filename limitations. At least this way, we're not held back by
the least common denominator (DOS), especially since few people
use DOS nowadays.
- [All] The documentation has been updated to explain the behaviour of
the -b option in more detail.

Version 1.7.8	8 April 2008
- [Mac OS X] Fixed corrupted portion of config.h that only shows up when
compiling under Mac OS X (sorry - I don't know what happened)

Version 1.7.7	1 April 2008
- [Mac OS X] Added support for compiling tofrodos under Mac OS X.
- [FreeBSD] Added support for compiling tofrodos under FreeBSD.
- [OpenBSD] Added support for compiling tofrodos under OpenBSD (not tested).
- [All] Fixed typos in documentation (Unix man page and HTML version).

Version 1.7.6	15 March 2005
- [All systems] New option: -l allows you to send the error messages to
a log file. It's particularly convenient for systems with less powerful
command line shells that do not allow you to easily redirect stderr.
- [All systems] Fixed: all error and verbose messages are now sent to
stderr or the error log file (if -l is given).
- [Windows] Older versions of compilers like Borland 4.X and 5.0,
Watcom 10.X and Visual C++ 6.0 no longer have makefiles. Use the latest
versions; all these compilers are now available free of charge from
their vendors so using the latest version to compile tofrodos should
not be a great hardship.
- [All systems] Other minor fixes.

Version 1.7	26 November 2003
- [Linux/Unix] Bug fix: now handles symbolic link arguments correctly.
This bug only affects Unix-like systems (like Linux, FreeBSD, OpenBSD,
etc).
- [Linux] tofrodos now uses mkstemp() to create the temporary file to
avoid a race between file name creation and opening which may occur
when using mktemp(). If you use a non-Linux system, and have
mkstemp(), you can take advantage of this by defining HAVE_MKSTEMP
in config.h (defined by default for Linux only).
- [Linux/Unix] "make install" now installs to /usr/bin by default (you
can still change this easily) since Linux distributions using tofrodos
appear to use this value. (Makes it easier for maintainers of those
distributions.)
- [All systems] Made some error messages a bit more informative.
- [All systems] Verbose mode is slightly more informative.
- [Windows] Added support for compiling with the MinGW (GNU) C Compiler
on Win32 systems.
- [All systems] Added an HTML version of the manual page. Useful for
systems that don't have a built-in facility to read a man page (like
MSDOS and Windows).

Version 1.6	1 July 2002
- Added support for LCC-Win32 and BCC 5.5

Version 1.5	19 June 2002
- Minor fixes to documentation.
- Added support for the Digital Mars C/C++ compiler (Win32 console mode)

Version 1.4	16 March 1999
- Fixed bug when using -b on a file not in the current directory.
- Added RPM support.

Version 1.3	8 October 1997
- Added new option (-p) to preserve file ownership and time.
- Added support for Win32 compilation and some compilers under DOS and
Windows.

Version 1.2	5 April 1997
- (Internal version - not publicly released.)

Version 1.1	16 December 1996
- Fixed bug in creation of temporary filenames on Linux.
- Fixed bug in creation of backup filenames on Linux.

Version 1.0	22 June 1996
- Initial version.

## <a id="copyright"></a>Copyright and Licence

The program and its accompanying files and documentation are  
Copyright 1996-2026 Christopher Heng. All rights reserved.

The included `getopt.c` and `getopt.h` are distributed under the terms of the
GNU General Public License Version 3, a copy of which is enclosed in
this package in the file COPYING-GPL3.txt

The other files are distributed under the terms of the GNU General Public License
Version 2, which you can find in the file COPYING-GPL2.txt.

You can get the latest version of tofrodos from its repository at:  
	https://github.com/ChristopherHeng/tofrodos

Bugs can be reported there in the usual way.
