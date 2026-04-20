/*
	windows/check_and_save_file_info.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <windows.h>	// GetFileInformationByHandle(), BY_HANDLE_FILE_INFORMATION, CreateFile(), CloseHandle()
#include <sys/types.h>	// needed for sys/stat.h
#include <sys/stat.h>	// stat()

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
	int * use_copy_and_convert_methodp, int * need_to_make_writeablep )
{
	struct stat statbuf ;
	HANDLE fh ;
	BY_HANDLE_FILE_INFORMATION file_info ;
	int retval ;

	do {
		retval = -1 ;
		fh = INVALID_HANDLE_VALUE ;

		/* get the file information */
		if (stat( filename, &statbuf )) {
			/* couldn't stat the file. */
			emsg( EMSG_ACCESSFILE, filename );
			break ;
		}
		/* save the mode */
		*origfilemodep = statbuf.st_mode ;
		/* save the file times for restoration later */
		filetimebufp->actime = statbuf.st_atime ;
		filetimebufp->modtime = statbuf.st_mtime ;

		// we actually don't use this in Windows and DOS (always zero)
		*ownerp = statbuf.st_uid ;
		*groupp = statbuf.st_gid ;

		// For Windows, there are no group/everyone permissions; just the user.
		// So we check the mode directly.
		if (!(statbuf.st_mode & S_IWUSR)) { // not writeable
			*need_to_make_writeablep = 1 ;
			if (!forcewrite) {
				emsg( EMSG_NOTWRITEABLE, filename );
				break ;
			}
		}
		else {
			*need_to_make_writeablep = 0 ;
		}

#if 0	// This, sadly, does not work in Visual Studio 2026 / UCRT: it always reports 1.
		if (statbuf.st_nlink > 1) {
			// file has more than one hard link
			*has_multiple_hard_linksp = 1 ;
		}
		else {
			*has_multiple_hard_linksp = 0 ;
		}
#endif

		// find out the number of hard links through the Windows API
		fh = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ,
			NULL, // lpSecurityAttributes
			OPEN_EXISTING, // dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
			NULL );
		if (fh == INVALID_HANDLE_VALUE) {
			// don't use EMSG_OPENFILE but be more specific since we're opening at this stage
			// to get file info. (This makes it easier to pinpoint the location of/reason for failure.)
			emsg( EMSG_FILEINFO, filename );
			break ;
		}
		if (GetFileInformationByHandle( fh, &file_info )) {
			if (file_info.nNumberOfLinks > 1) {
				*use_copy_and_convert_methodp = 1 ;
			}
		}
		else {
			emsg( EMSG_FILEINFO, filename );
			break ;
		}
		// Since we're doing this, in theory, we needn't have used stat() earlier. But I can't
		// be bothered to convert the Windows FILETIME to time_t, and translate the Windows file
		// attributes into the mode_t equivalent.

		retval = 0 ;

	} while(0) ;

	if (fh != INVALID_HANDLE_VALUE) {
		CloseHandle( fh );
	}
	return retval ;
}
