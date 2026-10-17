/*
	posix/resolve_filename_and_convert.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.

	resolve_filename_and_convert() for Unix-type systems.
*/

#include "config.h"

#include <assert.h>	// static_assert()
#include <limits.h>	// REAL_PATH
#include <stdint.h> // SIZE_MAX
#include <stdlib.h>	// free(), realpath()
#include <sys/stat.h> // lstat()

#include "emsg.h"
#include "safely_add_size_t_operands.h"
#include "tofrodos.h"
#include "utility.h" // xmalloc()

#if defined(UNIX)

/*
	On a Unix-type system (including Linux and BSD systems), it is
	possible for a given filename to be merely a symlink. This function
	obtains the real filename given the symlink, and calls
	process_file() with the real file name. It simply calls process_file()
	if the filename is not a symlink.

	Returns: whatever process_file() returns. (0 on success, -1 on
	error. Error messages will be displayed on error before returning.
*/
int resolve_filename_and_convert ( char * filename )
{
	struct stat	statbuf ;
	char *		realfilepath ;
	int			ret_val ;

	do {
		ret_val = -1 ;
		realfilepath = NULL ;

		/* get the file information */
		if (lstat( filename, &statbuf )) {
			/* couldn't stat the file. */
			emsg( EMSG_ACCESSFILE, filename );
			break ;
		}

		if (S_ISLNK(statbuf.st_mode)) {
			if ((realfilepath = malloc( PATH_MAX )) == NULL) {
				emsg( EMSG_NOMEM );
				break ;
			}
			if (realpath( filename, realfilepath ) == NULL) {
				emsg ( EMSG_SYMLINK, filename );
				break ;
			}
			if (verbose) {
				emsg( VERBOSE_SYMLINKSRC, filename, realfilepath );
			}
			ret_val = process_file( realfilepath );
		}
		else {
		/* If we reach here, "filename" is not a symbolic link */
			ret_val = process_file( filename );
		}
	} while (0);

	if (realfilepath != NULL) {
		free( realfilepath );
	}
	return ret_val ;
}

#endif // defined(UNIX)
