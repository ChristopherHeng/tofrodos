/*
	posix/resolve_filename_and_convert.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.

	resolve_filename_and_convert() for Unix-type systems.
*/

#include "config.h"

#include <stdlib.h>	// free()
#include <unistd.h>	// readlink(),
#include <sys/stat.h> // lstat()

#include "emsg.h"
#include "tofrodos.h"
#include "utility.h" // xmalloc()

#if defined(UNIX)

/*
	On a Unix-type system (including Linux and BSD systems), it is
	possible for a given filename to be merely a symlink. This function
	obtains the real filename given the symlink, and calls
	openandconvert() with the real file name. It simply calls openandconvert()
	if the filename is not a symlink.

	Returns: whatever openandconvert() returns. (0 on success, -1 on
	error. Error messages will be displayed on error before returning.
*/
int resolve_filename_and_convert ( char * filename )
{
	struct stat	statbuf ;
	char *		realfilepath ;
	int			len ;
	int			err ;

	/* get the file information */
	if (lstat( filename, &statbuf )) {
		/* couldn't stat the file. */
		emsg( EMSG_ACCESSFILE, filename );
		return -1 ;
	}

	if (S_ISLNK(statbuf.st_mode)) {
		/*
			Get the real filename for symbolic links.
			Note: the S_ISLNK() macro is supposed to exist in sys/stat.h
			Early Unices may not have this macro. If it does not exist, you
			may have to define it yourself, a la S_ISDIR() and family.
			eg, #define S_ISLNK(x) (((x) & S_IFMT) == S_IFLNK)
			or something like that.
		*/

		/* for symbolic links, st_size contains the length of the pathname sans terminating null byte */
		if (statbuf.st_size == 0) {
			/* There's a report somewhere of a discovery that macOS returns st_size == 0 for "/dev/stdin" when it
			is a symlink to "fd/0". I'm not sure if it is a valid report, but let's play it safe. */
			emsg ( EMSG_SYMLINK, filename );
			return -1 ;
		}
		realfilepath = xmalloc( statbuf.st_size + 1 );

		if ((len = readlink( filename, realfilepath, statbuf.st_size )) != -1) {

			/* got to null terminate the string - there is always space because */
			/* we passed readlink() the size of the buffer less 1. */
			realfilepath[len] = '\0' ;
			if (verbose) {
				emsg( VERBOSE_SYMLINKSRC, filename, realfilepath );
			}
			err = process_file( realfilepath );
		}
		else {
			emsg( EMSG_SYMLINK, filename );
			err = -1 ;
		}
		free ( realfilepath );
		return err ;
	}
	/* If we reach here, "filename" is not a symbolic link */
	return process_file( filename );
}

#endif // defined(UNIX)
