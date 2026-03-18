/*
	process_file.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"
#include <errno.h>	// errno
#include <stdio.h>	// close(), fdopen(), fopen(), remove(), fclose(), rename()
#include <stdlib.h>	// free(), mkstemp() on some systems
#include <string.h>	// strerror(), memset()
#include <sys/types.h>
#include <sys/stat.h>	// chmod(), stat()

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	// chown(), mkstemp() on some systems
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/utime.h>
extern int mkstemp ( char * templ );
#else
#include <utime.h>
#endif

#if defined(MSDOS) || defined(WIN32)
#include <fcntl.h>	// O_BINARY
#include <io.h>		// chmod(), setmode()
#endif

#include "emsg.h"
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
#if !defined(UNIX)
static int checkmode ( char * filename, unsigned short * origfilemodep,
	struct utimbuf * filetimebufp );
#else
static int checkmode ( char * filename, unsigned short * origfilemodep,
	struct utimbuf * filetimebufp, uid_t * ownerp, gid_t * groupp );
#endif

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
	unsigned short	origfilemode ;	/* file mode of original file */
	struct utimbuf	filetimebuf ;
#if defined(UNIX)
	uid_t			ownerid ;
	gid_t			groupid ;
#endif

	// make sure we initialise so that we know what to free/close/etc later
	tempfiledes = -1 ;
	bakfilename = NULL ;
	tempfilename = NULL ;
	infp = NULL ;
	outfp = NULL ;
	err = 0 ;

	do {
		if (filename != NULL) { /* stdin is not redirected */

		// check for appropriate permissions, and save it in origfilemode
#if !defined(UNIX) 
			if (checkmode( filename, &origfilemode, &filetimebuf )) {
				err = -1 ;
				break ;
			}
#else
			if (checkmode( filename, &origfilemode, &filetimebuf, &ownerid, &groupid )) {
				err = -1 ;
				break ;
			}
#endif

			// generate the mkstemp() template in the same directory as filename
			// and if overwrite == 0 (user wants a backup file), generate the
			// backup filename as well
			make_filenames( filename,  &tempfilename, (overwrite ? NULL : &bakfilename) );

		    /* create the temporary file */
		    if ((tempfiledes = mkstemp( tempfilename )) == -1) {
				emsg( EMSG_NOTEMPNAME, filename );
				err = -1 ;
				break ; // tempfilename and bakfilename will be freed by the cleanup code later
		    }

		    /* open the filename as the input file */
		    if ((infp = fopen( filename, INFILEMODE )) == NULL) {
				emsg( EMSG_OPENFILE, filename );
				close( tempfiledes );
				tempfiledes = -1 ;
				remove( tempfilename );
				err = -1 ;
				break ; // tempfilename and bakfilename will be freed by the cleanup code later
		    }
		    /* associate the current_input_filename with the filename for error messages */
		    current_input_filename = filename ;

#if defined(MSDOS) || defined(WIN32)
			/*
				Set the temporary file descriptor to binary mode. This may or may not
				be necessary since we fdopen() it later in binary anyway. This is
				just in case the C library of one of the compilers require the mode
				to be consistent between open() and fdopen().
			*/
		    setmode( tempfiledes, O_BINARY );
#endif
		    /* open the temp file as the output file */
			if ((outfp = fdopen( tempfiledes, OUTFILEMODE )) == NULL) {
				emsg( EMSG_CREATETEMP, tempfilename, filename );
				fclose( infp );
				infp = NULL ;
		    	close( tempfiledes );
				tempfiledes = -1 ;
		    	remove( tempfilename );
				err = -1 ;
				break ; // tempfilename and bakfilename will be freed by the cleanup code later
		    }
		    tempfiledes = -1 ; // it's now associated with outfp and it'll be closed when we fclose that

		} /* if filename != NULL */
		else { /* filename == NULL, ie stdin is redirected */
			infp = stdin ;
			outfp = stdout ;
			current_input_filename = "stdin" ;

			/* not needed, but we do this for the record, and for fussy compilers */
			origfilemode	= 0 ;
			memset( &filetimebuf, 0, sizeof( struct utimbuf ) );

#if defined(UNIX)
			ownerid	= groupid	= 0 ;
#endif

		}

		/* do the conversion */
		err = convert_file( infp, outfp );

		/* close the files */
		fclose( infp );
		fclose( outfp );
		infp = NULL ;
		outfp = NULL ;
		tempfiledes = -1 ;

		if (filename != NULL) { /* stdin was not redirected */

		    if (err) { /* there was an error */
	        	/* delete the temp file since we've already created it */
	        	remove ( tempfilename );
				break ; // tempfilename and bakfilename will be freed by the cleanup code later
		    }

		    if (!overwrite) {

#if defined(MSDOS) || defined(WIN32)
				/* delete any backup file of the same name first, since a rename() does not delete it automatically */
				/* on DOS and Windows */
	            chmod( bakfilename, S_IRUSR|S_IWUSR );	/* make it writeable (in case it's not) so that it can be deleted */
				remove( bakfilename );	/* don't check for error returns since the file may not even exist in the first place */
#endif

				/* rename the original file to the back up name */
				if (rename( filename, bakfilename )) {
					emsg( EMSG_RENAMEBAK, filename, bakfilename, strerror( errno ) );
				}
		    }

#if defined(MSDOS) || defined(WIN32)
			/* we need to delete the original file because a rename() operation will not */
			/* automatically delete it for us on DOS and Windows the way it does on POSIX systems */
			if (overwrite) { /* if we do not need to back up the original file */
	            chmod( filename, S_IRUSR|S_IWUSR );	/* make it writeable (in case it's not) so that it can be deleted. */
				remove( filename );	/* delete the original file */
				/* we don't check for error returns for this, since any error message about its failure will just */
				/* confuse the user. "What? Why is it deleting my file?" If this fails, the next rename() will fail too */
				/* since rename() on Windows will not delete the target automatically, and the error message will from the */
				/* failed rename() will tell the user what happened. */
		    }
#endif

		    /* rename the temp file to the original file name */
		    if (rename( tempfilename, filename )) {
		    	emsg( EMSG_RENAMETMP, tempfilename, filename, strerror( errno ) );
		    }

			if (preserve) {
				/* change to the original file time */
				utime( filename, &filetimebuf );
#if defined(UNIX)
				/* Change the owner to the owner of the original file. */
				/* We ignore errors since the user might simply want */
				/* to use -p to set the file time, and not being root, */
				/* chown() will fail on Linux. However, we issue an error */
				/* message if the user wants verbosity. */
				if (chown( filename, ownerid, groupid ) && verbose)
					emsg( EMSG_CHOWN, filename );
#endif
			}

		    /* change the file mode to reflect the original file mode */
	        chmod( filename, origfilemode );

		}	/* stdin was not redirected */
	} while (0) ;

	// Clean up of memory allocations only.
	// Closing of files are done above to make sure files are closed in the proper order
	// (eg, tempfiledes is closed only if it has not already been closed via fclose of the
	// corresponding FILE pointer), and that the temporary file is deleted if need be.
	if (tempfilename != NULL) {
		free( tempfilename );
	}
	if (bakfilename != NULL)
		free( bakfilename );

	return err ;
}

/*
	checkmode

	Checks that the file we are supposed to convert is indeed
	writeable. We don't really need for it to be writeable, since
	we actually open a new file and eventually delete the current
	file.

	However, if a file is marked not-writeable, we should at least
	respect the user's choice and abort unless --force is in effect.

	At the same time we also save the current mode of the file
	so that we can set the converted file to the same mode. The
	value is saved in the variable pointed to by origfilemodep.

	Returns: 0 on success, -1 on error.

	If -1 is returned, it could mean one of few things:
	1) some component of the path was not valid (directory or the file
	itself) (DOS/Unix) or search permission was denied (Unix)
	2) the file is not readable
	3) the file is not writeable and forcewrite is zero.
	An error message is displayed on error.
*/
#if !defined(UNIX)
static int checkmode ( char * filename, unsigned short * origfilemodep,
	struct utimbuf * filetimebufp )
#else
static int checkmode ( char * filename, unsigned short * origfilemodep,
	struct utimbuf * filetimebufp, uid_t * ownerp, gid_t * groupp )
#endif
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
#if defined(UNIX)
	/* save the owner and group id */
	*ownerp = statbuf.st_uid ;
	*groupp = statbuf.st_gid ;
#endif
	/* check if file can be read - this is actually redundant for */
	/* DOS systems. */
	if (!(statbuf.st_mode & S_IRUSR)) { /* not readable */
		emsg( EMSG_NOTREADABLE, filename );
		return -1 ;
	}
	/* check if file can be written to, if forcewrite is 0 */
	if (!forcewrite && !(statbuf.st_mode & S_IWUSR)) { /* not writeable */
		emsg( EMSG_NOTWRITEABLE, filename );
		return -1 ;
	}
	return 0 ;
}

