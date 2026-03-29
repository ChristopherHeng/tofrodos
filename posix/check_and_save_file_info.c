/*
	check_and_save_file_info.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	// access()
#endif

#include <sys/types.h>	// needed for sys/stat.h and getuid()
#include <sys/stat.h>	// stat()

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include "emsg.h"

#define	DECLARE_CHECK_AND_SAVE_FILE_INFO
#include "tofrodos.h"

/*
	check_and_save_file_info

	Checks that the file we are supposed to convert is indeed
	writeable.

	That said, if the file has only one hard link, we don't really
	need for it to be writeable, since we actually open a new file
	and eventually delete the current file. However, even in this
	case, if a file is marked not-writeable, we should at least
	respect the user's choice and abort unless --force is in effect.
	If the file has multiple hard links, we do however need it to
	be writeable, since we will be writing directly to it. So this
	consistency in behaviour is good.

	At the same time we also save the current mode of the file
	so that we can set the converted file to the same mode. The
	value is saved in the variable pointed to by origfilemodep.

	On Windows and Unix-type systems, if the file has multiple hard
	links, we save that information in has_multipe_hard_linksp.

	On Windows and Unix-type systems, if the file has no write
	permissions for the current user but is nonetheless owned
	by him/her, and the --force flag was set, need_to_makewriteablep
	is set to 1, else it is set to zero.

	Returns: 0 on success, -1 on error.

	If -1 is returned, it could mean one of few things:
	1) some component of the path was not valid (directory or the file
	itself) (DOS/Unix) or search permission was denied (Unix)
	2) the file is not readable
	3) the file is not writeable and forcewrite is zero.
	4) The file is not writeable and forcewrite is 1, but it is
	not owned by the user.
	An error message is displayed on error.
*/

int check_and_save_file_info ( char * filename, mode_t * origfilemodep,
	struct utimbuf * filetimebufp, uid_t * ownerp, gid_t * groupp,
	int * has_multiple_hard_linksp, int * need_to_make_writeablep )
{
	struct stat statbuf ;
	uid_t user_id ;

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

		/* save the owner and group id */
	*ownerp = statbuf.st_uid ;
	*groupp = statbuf.st_gid ;

	if (statbuf.st_nlink > 1) {
		// file has more than one hard link
		*has_multiple_hard_linksp = 1 ;
	}
	else {
		*has_multiple_hard_linksp = 0 ;
	}

	// check if file can be read by the current user
	// on Windows/MSDOS, this check is pointless; access() on Windows only checks the read-only flag,
	// and not the security settings.
	if (access( filename, R_OK )) {
		emsg( EMSG_NOTREADABLE, filename );
		return -1 ;
	}

	if (access( filename, W_OK)) { // not writeable
		user_id = getuid() ;
		*need_to_make_writeablep = 1 ;
		if (user_id != statbuf.st_uid) { // not the owner of the file
			if (forcewrite) {
				emsg( EMSG_NOTWRITEABLE_CANNOT_FORCE, filename );
			}
			else {
				emsg( EMSG_NOTWRITEABLE_NOT_OWNER, filename );
			}
			return -1 ;
		}
		else { // owner of the file
			if (!forcewrite) {
				emsg( EMSG_NOTWRITEABLE, filename );
				return -1 ;
			}
			// else --force was used
		}
	}
	else {
		*need_to_make_writeablep = 0 ;
	}

	return 0 ;
}

