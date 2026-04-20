/*
	process_file.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"
#include <errno.h>	// errno
#include <stdio.h>	// close(), fdopen(), fopen(), remove(), fclose(), rename()
#include <stdlib.h>	// free(), mkstemp() on some systems
#include <string.h>	// strerror(), memset()
#include <sys/types.h>	// needed for sys/stat.h
#include <sys/stat.h>	// chmod(), stat()

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	// chown(), mkstemp() on some systems, access()
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/utime.h>
extern int mkstemp ( char * templ );
#else
#include <utime.h>
#endif

#if defined(MSDOS) || defined(WIN32)
#include <fcntl.h>	// O_BINARY
#include <io.h>		// chmod(), _setmode()
#endif

#include "copy_file.h"
#include "emsg.h"
#define	DECLARE_CHECK_AND_SAVE_FILE_INFO
#include "tofrodos.h"
#include "utility.h"

// macros
#if defined(MSDOS) || defined(WIN32)
#define	INFILEMODE	"rb"
#define	OUTFILEMODE	"wb"
#else
#define	INFILEMODE	"r"
#define	OUTFILEMODE	"w"
#endif

// local functions
#if defined(UNIX) || defined(WIN32)
static inline int set_up_for_copy_and_convert( char * filename, char * tempfilename,
	mode_t origfilemode, int need_to_make_writeable, FILE ** infp_p, FILE ** outfp_p );
static inline void clean_up_for_copy_and_convert( int has_error_in_conversion, char * filename, char * tempfilename,
	char * bakfilename, mode_t origfilemode, struct utimbuf	* filetimebufp, uid_t ownerid, gid_t groupid );
#endif
static inline int set_up_for_convert_and_rename( char * filename, char * tempfilename,
	int * tempfiledesp, FILE ** infp_p, FILE ** outfp_p );
static inline void clean_up_for_convert_and_rename( int has_error_in_conversion, char * filename, char * tempfilename,
	char * bakfilename, mode_t origfilemode, struct utimbuf	* filetimebufp,
	uid_t ownerid, gid_t groupid );

/*
	process_file

	Called to open the files and convert the contents. If you want
	it to convert stdin to stdout, call it with NULL as the filename
	argument; otherwise pass the function the name of the input file.

	Returns: 0 on success, -1 on error. Error messages will be
        displayed on error before returning.
*/
int process_file ( char * filename )
{
	FILE *			infp ;
	FILE *			outfp ;
	int				err ;
	char *			bakfilename ;
	char *			tempfilename ;
    int				tempfiledes;
	mode_t			origfilemode ;
	struct utimbuf	filetimebuf ;
	uid_t			ownerid ;
	gid_t			groupid ;
	int				use_copy_and_convert_method ;
	int				need_to_make_writeable ;

	// make sure we initialise so that we know what to free/close/etc later
	tempfiledes = -1 ;
	bakfilename = NULL ;
	tempfilename = NULL ;
	infp = NULL ;
	outfp = NULL ;
	err = 0 ;
	use_copy_and_convert_method = 0 ; // default to convert and rename

	do {
		if (filename != NULL) { /* stdin is not redirected */

			if (check_and_save_file_info( filename, &origfilemode, &filetimebuf, &ownerid, &groupid,
				&use_copy_and_convert_method, &need_to_make_writeable )) {
				err = -1 ;
				break ;
			}

			// generate the mkstemp() template in the same directory as filename
			// and if overwrite == 0 (user wants a backup file), generate the
			// backup filename as well
			if ((err = make_filenames( filename,  &tempfilename, (overwrite ? NULL : &bakfilename),
				&use_copy_and_convert_method )) == -1) {
				break ;
			}

		    /* create the temporary file */
		    if ((tempfiledes = mkstemp( tempfilename )) == -1) {
				emsg( EMSG_NOTEMPNAME, filename );
				err = -1 ;
				break ; // tempfilename and bakfilename will be freed by the cleanup code later
		    }

#if defined(MSDOS) || defined(WIN32)
			/*
				Set the temporary file descriptor to binary mode. This may or may not
				be necessary since we fdopen() it later in binary anyway. This is
				just in case the C library of one of the compilers require the mode
				to be consistent between open() and fdopen().

				Note to self: this _setmode() is different from BSD's setmode().
			*/
		    _setmode( tempfiledes, O_BINARY );
#endif

#if defined(UNIX) || defined(WIN32)
			if (use_copy_and_convert_method) {
				close( tempfiledes );
				tempfiledes = -1 ;
				if (verbose) {
					emsg( VERBOSE_ALTMETHOD, filename );
				}
				err = set_up_for_copy_and_convert( filename, tempfilename,
						origfilemode, need_to_make_writeable, &infp, &outfp );
			}
			else {
				// if we reach this point, it's not a file with more than one hard link
				err = set_up_for_convert_and_rename( filename, tempfilename, &tempfiledes, &infp, &outfp );
			}
#else	// MSDOS
			err = set_up_for_convert_and_rename( filename, tempfilename, &tempfiledes, &infp, &outfp );
#endif

		} /* if filename != NULL */
		else { /* filename == NULL, ie stdin is redirected */
			infp = stdin ;
			outfp = stdout ;
			current_input_filename = "stdin" ;

			// not needed (defensive); also silences erroneous compiler warnings later
			origfilemode	= 0 ;
			memset( &filetimebuf, 0, sizeof( struct utimbuf ) );
			ownerid = 0 ;
			groupid = 0 ;
			need_to_make_writeable = 0 ;
		}
		if (err) {
			break ;
		}

		/* do the conversion */
		err = convert_file( infp, outfp );

		/* close the files */
		if (infp != NULL) {
			fclose( infp );
			infp = NULL ;
		}
		if (outfp != NULL) {
			fclose( outfp );
			outfp = NULL ;
		}

		if (filename != NULL) { /* stdin was not redirected */

#if defined(UNIX) || defined(WIN32)
			if (use_copy_and_convert_method) {
				clean_up_for_copy_and_convert( err, filename, tempfilename, bakfilename,
					origfilemode, &filetimebuf, ownerid, groupid );
			}
			else {
				clean_up_for_convert_and_rename( err, filename, tempfilename, bakfilename,
					origfilemode, &filetimebuf, ownerid, groupid );
			}
#else	// MSDOS only
			clean_up_for_convert_and_rename( err, filename, tempfilename, bakfilename,
				origfilemode, &filetimebuf, ownerid, groupid );
#endif
		}	/* stdin was not redirected */

	} while (0) ;

	// Clean up of memory allocations only.
	// Closing of files are done above to make sure files are closed in the proper order
	// (eg, tempfiledes is closed only if it has not already been closed via fclose of the
	// corresponding FILE pointer), and that the temporary file is deleted if need be,
	// or left as-is for recovery purposes after an error.
	if (tempfilename != NULL)
		free( tempfilename );
	if (bakfilename != NULL)
		free( bakfilename );

	return err ;
}

#if defined(UNIX) || defined(WIN32)
static inline int set_up_for_copy_and_convert( char * filename, char * tempfilename,
	mode_t origfilemode, int need_to_make_writeable, FILE ** infp_p, FILE ** outfp_p )
{
	int copy_retval ;
	int retval ;

	do {
		retval = 0 ;
		copy_retval = copy_file( filename, tempfilename );
		switch( copy_retval ) {
			case cp_dest_write_error:
				emsg( EMSG_WRITETEMP, tempfilename );
				retval = -1 ;
				break ;
			case cp_source_read_error:
				emsg( EMSG_READFILE, filename );
				retval = -1 ;
				break ;
			case cp_dest_open_error:
				emsg( EMSG_OPENTEMP, tempfilename );
				retval = -1 ;
				break ;
			case cp_source_open_error:
				emsg( EMSG_OPENFILE, filename );
				retval = -1 ;
				break ;
			case cp_no_error:
				break ;
			default:
				// internal error
				emsg( EMSG_INTERNAL, "Invalid return value from copy_file()\n" );
				retval = -1 ; // we don't abort() though, just skip this file (unless --exit-on-error is on)
				break ;
		}
		if (retval) {
			break ; // break out of do/while(0);
		}
		if ((*infp_p = fopen( tempfilename, INFILEMODE )) == NULL) {
			emsg( EMSG_OPENTEMP, tempfilename );
			remove( tempfilename );
			retval = -1 ;
			break ;
		}
		if (need_to_make_writeable) {
			// make output file writeable
			if (chmod( filename, origfilemode | S_IWUSR )) {
				emsg( EMSG_MAKEWRITEABLE, filename );
				fclose( *infp_p );
				*infp_p = NULL ;
				remove( tempfilename );
				retval = -1 ;
				break ;
			}
		}
		if ((*outfp_p = fopen( filename, OUTFILEMODE )) == NULL) {
			emsg( EMSG_OPENFILE, filename );
			fclose( *infp_p );
			*infp_p = NULL ;
			remove( tempfilename );
			retval = -1 ;
			break ;
		}
		// although filename is not technically the input filename at this
		// point (since we have copied its contents to tempfilename), from the
		// point of view of the user, it is the filename they supplied to Tofrodos.
		// Since current_input_filename is only used for error messages, we should use
		// that instead of the temp file name which will mystify the user if used.
		current_input_filename = filename ;
	} while (0) ;

	return retval ;
}
#endif

static inline int set_up_for_convert_and_rename( char * filename, char * tempfilename,
	int * tempfiledesp, FILE ** infp_p, FILE ** outfp_p )
{
	int retval ;

	do {
		retval = 0 ;

		/* open the filename as the input file */
		if ((*infp_p = fopen( filename, INFILEMODE )) == NULL) {
			emsg( EMSG_OPENFILE, filename );
			close( *tempfiledesp );
			remove( tempfilename );
			retval = -1 ;
			break ; // tempfilename and bakfilename will be freed by the cleanup code later
		}
		/* associate the current_input_filename with the filename for error messages */
		current_input_filename = filename ;

		/* open the temp file as the output file */
		if ((*outfp_p = fdopen( *tempfiledesp, OUTFILEMODE )) == NULL) {
			emsg( EMSG_CREATETEMP, tempfilename, filename );
			fclose( *infp_p );
			*infp_p = NULL ;
			close( *tempfiledesp );
			remove( tempfilename );
			retval = -1 ;
			break ; // tempfilename and bakfilename will be freed by the cleanup code later
		}

	} while (0);

	// regardless of whether there's an error or not, tempfiledes is no longer valid
	// If there was an error, we would have closed the handle. If there's no error, we would
	// have assigned the handle to the fdopen() FILE.
	*tempfiledesp = -1 ;

	return retval ;
}

#if defined(UNIX) || defined(WIN32)
static inline void clean_up_for_copy_and_convert( int has_error_in_conversion, char * filename, char * tempfilename,
	char * bakfilename, mode_t origfilemode, struct utimbuf	* filetimebufp, uid_t ownerid, gid_t groupid )
{
#if defined(WIN32)
	UNUSED_VARIABLE( ownerid );
	UNUSED_VARIABLE( groupid );
#endif

	do {
		// If has_error_in_conversion is non-zero, it may mean that we may have partially converted the original file.
		// We don't simply copy_file() back the backup copy (kept in tempfilename) because we can't guarantee that will
		// succeed either (after all, it's because we failed to write to that file in the first place that brought us to this
		// situation).
		// So we will just tell the user where to get the original file content (in tempfilename). We aren't even going
		// to rename tempfilename to the backup filename, because that may mean that we wipe out an existing file of that name.
		if (has_error_in_conversion) {
			emsg( EMSG_CONVERT_MOREINFO, filename, tempfilename );
			break ;
		}
		// restore the file's original file mode (may have been changed by the default umask on Unix-type systems when we fopen() it);
		// this will automatically reset the file's write permissions to what it was before as well
		chmod( filename, origfilemode );
		// if a backup file was requested, we need to rename the temp file to the backup name
		if (overwrite) { // default (ie, --overwrite)
			// we don't want a backup file, so delete it
			remove( tempfilename );
		}
		else { // --backup
#if defined(WIN32)
			if (access( bakfilename, F_OK ) == 0) {
				// bakfilename already exists; need to delete it because rename() on Windows will not remove target
		        chmod( bakfilename, S_IRUSR|S_IWUSR );	// make it writeable (in case it's not)
				remove( bakfilename );
			}
#endif
			// a backup file was requested; rename the temp file to the backup name
			if (rename( tempfilename, bakfilename )) {
				// if there's an error, just print the info to the user so that he/she can
				// manually rename the file
				emsg( EMSG_RENAME_TMP_AS_BAK, tempfilename, bakfilename, strerror( errno ) );
			}
			// try to give the backup file the same mode, owner and time as the original file
			// though we ignore errors if we fail, since it's only the backup file
			chmod( bakfilename, origfilemode );
			utime( bakfilename, filetimebufp );
#if defined(UNIX)
			// somehow, prefixing (void) does not seem to silence the -Wunused-result warning
			// for chown() for gcc 13.3.0, so we have to temporarily disable it
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
			(void) chown( bakfilename, ownerid, groupid );
#pragma GCC diagnostic pop
#endif
		}
		// if --preserve was specified, we need to restore the file time
		if (preserve) {
			// --preserve; we need to restore the file time
			// since we ovewrote the original file, we don't have to restore the owner and group
			if (utime( filename, filetimebufp )) {
				emsg( EMSG_UTIME, filename );
			}
		}
	} while (0) ;
	return ;
}
#endif

static inline void clean_up_for_convert_and_rename( int has_error_in_conversion, char * filename, char * tempfilename,
	char * bakfilename, mode_t origfilemode, struct utimbuf	* filetimebufp,
	uid_t ownerid, gid_t groupid )
{
#if defined(MSDOS) || defined(WIN32)
	UNUSED_VARIABLE(ownerid);
	UNUSED_VARIABLE(groupid);
#endif

	do {
	    if (has_error_in_conversion) {
	    	// there was an error while writing to the temporary file
	    	// undo everything by deleting the temp file (since it has already been created)
	    	remove ( tempfilename );
			break ; // tempfilename and bakfilename will be freed by the cleanup code later
	    }

	    if (!overwrite) { // --backup specified
#if defined(MSDOS) || defined(WIN32)
			// delete any backup file of the same name first, since a rename() does not delete it automatically
			// on DOS and Windows
	        chmod( bakfilename, S_IRUSR|S_IWUSR );	// make it writeable (in case it's not) so that it can be deleted
			remove( bakfilename );	// don't check for error returns since the file may not even exist in the first place
#endif
			/* rename the original file to the back up name */
			if (rename( filename, bakfilename )) {
				emsg( EMSG_RENAMEBAK, filename, bakfilename, strerror( errno ) );
			}
	    }
	    else { // don't back up the original file
#if defined(MSDOS) || defined(WIN32)
			// we need to delete the original file because a rename() operation will not
			// automatically delete it for us on DOS and Windows the way it does on POSIX systems
	        chmod( filename, S_IRUSR|S_IWUSR );	// make it writeable (in case it's not) so that it can be deleted.
			remove( filename );	// delete the original file
			// we don't check for error returns for this, since any error message about its failure will just
			// confuse the user. "What? Why is it deleting my file?" If this fails, the next rename() will fail too
			// since rename() on Windows will not delete the target automatically, and the error message will from the
			// failed rename() will tell the user what happened.
#endif
			// this else clause is empty on POSIX systems
		}
	    // rename the temp file to the original file name
	    if (rename( tempfilename, filename )) {
	    	emsg( EMSG_RENAMETMP, tempfilename, filename, strerror( errno ) );
	    }

		if (preserve) { // --preserve
			/* change to the original file time */
			if (utime( filename, filetimebufp )) {
				emsg( EMSG_UTIME, filename );
			}
#if defined(UNIX)
			// Change the owner to the owner of the original file.
			if (chown( filename, ownerid, groupid )) {
				emsg( EMSG_CHOWN, filename );
			}
#endif
		}

	    // change the file mode to reflect the original file mode
	    chmod( filename, origfilemode );

	} while (0) ;

	return ;
}
