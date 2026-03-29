/*
	check_and_save_file_info.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <sys/types.h>	// needed for sys/stat.h and getuid()
#include <sys/stat.h>	// chmod(), stat()

#if defined(MSDOS) || defined(WIN32)
#include <io.h>		// access()
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include "emsg.h"

#define	DECLARE_CHECK_AND_SAVE_FILE_INFO
#include "tofrodos.h"

/*
	see the documentation in posix/check_and_save_file_info.c
*/

int check_and_save_file_info ( char * filename, mode_t * origfilemodep,
	struct utimbuf * filetimebufp, uid_t * ownerp, gid_t * groupp,
	int * has_multiple_hard_linksp, int * need_to_make_writeablep )
{
	struct stat statbuf ;

	/* get the file information */
	if (stat( filename, &statbuf )) {
		/* couldn't stat the file. */
		emsg( EMSG_ACCESSFILE, filename );
		return -1 ;
	}
	/* save the mode */
	*origfilemodep = statbuf.st_mode ;
	/* save the file times for restore later */
	filetimebufp->actime = statbuf.st_atime ;
	filetimebufp->modtime = statbuf.st_mtime ;

	// we actually don't use this in DOS
	*ownerp = statbuf.st_uid ;
	*groupp = statbuf.st_gid ;
	*has_multiple_hard_linksp = 0 ;

	/* check if file can be written to, if forcewrite is 0 */
	if (access( filename, W_OK )) { // not writeable
		*need_to_make_writeablep = 1 ;
		if (!forcewrite) {
			emsg( EMSG_NOTWRITEABLE, filename );
			return -1 ;
		}
	}
	else {
		*need_to_make_writeablep = 0 ;
	}

	return 0 ;
}

