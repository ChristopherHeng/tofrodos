# NAME

Tofrodos \- convert text files between DOS and Unix formats.

# SYNOPSIS

**fromdos** [*options*] [*file...*]  
**todos** [*options*] [*file...*]  

# DESCRIPTION

DOS (and Windows) plain ASCII or Unicode UTF\-8 text files traditionally have
a carriage return and line feed pair to mark the end of each line, while Unix\-type
systems (including Linux, macOS, FreeBSD, etc) use a line feed for that purpose.

**fromdos** converts such files from the DOS format to
the Unix format, while **todos** converts them in the other
direction.

The programs accept multiple filenames and wildcards as their arguments.
You can also use them in a pipe. If the program finds its input redirected,
it will take its input from stdin and send its output to stdout.

Note that **fromdos** and **todos** are actually the same program. The name under
which the program is run determines the default direction it converts (unless
overridden by an option). If you prefer, you can also rename it
to **unix2dos** (which will make it behave like **todos**) and **dos2unix** (which
will make it act as though it is **fromdos**).

# OPTIONS

**\-b, \-\-backup**

: Make a backup of the original file. It will have a ".bak" suffix appended to its name.
For example, "filename.ext" becomes "filename.ext.bak", and if there is already
a file named "filename.ext.bak" in that same directory, it will be silently replaced.

: Note: the program behaves differently if it is compiled for DOS (as compared to
being compiled for Linux, Windows, macOS, FreeBSD or other systems). In view of the filename
restrictions present on that system, where files have at most a single extension,
the original file extension (if any) will be replaced by ".bak". For example,
"filename.ext" becomes "filename.bak".

**\-d, \-\-dos2unix, \-\-fromdos**

: Convert from DOS to Unix. This forces the program to convert the file in
the specified direction. By default, if the program is named **fromdos** or
**dos2unix**, it will convert files to a Unix format. If it is named **todos** or **unix2dos**,
it will them to a DOS format. Using the "\-\-fromdos" option forces
the program to convert to a Unix format regardless of the program name. Likewise, using the
"\-\-todos" option forces it to convert to a DOS format.

**\-e, \-\-exit\-on\-error**

: Abort processing the rest of the files if there is an error in any file. Normally,
the program will simply skip to process the next file on the command line when it encounters
an error. This option causes it to abort instead.

**\-f, \-\-force**

: Convert the file even if it is not writeable (that is, it is read\-only). By default,
if the program finds that the file does not have write permission, it will not process it.
This option forces the conversion even if it is read\-only.

**\-h, \-\-help**

: Display a short help screen on the program usage and quit.

**\-l \<logfile\>, \-\-log=\<logfile\>**

: Log most error messages and all verbose messages to \<logfile\>.

: Note though that if your command line has an error, such as when you specify
an option that the program does not recognize, the error message will still be
printed on stderr (which usually means the screen), and not logged, since
at the time the program checks the command line, it has still not set up the log file.

**\-o, \-\-overwrite**

: Overwrite the original file (no backup). This is the default.

**\-p, \-\-preserve**

: Preserve file ownership and time on Unix\-type systems (like Linux, macOS, FreeBSD, etc).
On Windows and MSDOS, it only preserves the file time. Note that on Unix\-type systems, the
ownership will only be preserved if the program is run as root or via **sudo**, otherwise it
will just set the file time and silently fail the change of ownership.
If you want a warning message when the file ownership cannot be changed, set the verbose mode as well,
using the "\-\-verbose" option.

**\-u, \-\-unix2dos, \-\-todos**

: Convert from Unix to DOS. See the "\-\-fromdos" option above for more information.

**\-v, \-\-verbose**

: Display the current version and quit.

# EXIT CODE

Tofrodos terminates with an exit code of 0 on success and 1 on error.

If the program is invoked with multiple files on the command line, the default behaviour is to skip to the next
file in the list if an error is encountered with any file. In such a case, the exit code returned will the
status of the last file processed (ie, 0 on success, 1 on failure). If this is not desirable, use the
"\-\-exit\-on\-error" option, which will force the program to abort immediately with the appropriate
exit code on encountering an error.

# EXAMPLES

To convert a single file to Unix format (with line feeds):

	fromdos filetoconvert.txt

To convert multiple files, including wildcard characters, to DOS format (with carriage return/line feed pairs):

	todos readme.txt manual.html *.c *.h

If you want the original file backed up with a ".bak" extension:

	todos --backup filetoconvert.txt

To use the program in a pipe:

	cat dosversion.txt | fromdos | diff - unixversion.txt

# AUTHOR

The program and its accompanying files and documentation are:  
Copyright 1996\-2026 by Christopher Heng. All rights reserved.

The included "getopt.c" and "getopt.h" are distributed under the terms of the
GNU General Public License Version 3, and the remaining files under
the GNU General Public License Version 2.

The latest version of tofrodos can be obtained from  
<https://github.com/ChristopherHeng/tofrodos>

