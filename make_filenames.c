/*
	make_filenames.c
	Copyright 2021-2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <stdint.h> // SIZE_MAX
#include <stdlib.h>	// exit()
#include <string.h> // strlen(), strncpy(), strcat(), size_t

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	// access()
#endif

#include "emsg.h"
#include "tofrodos.h"
#include "utility.h"

#define	MKSTEMP_TEMPLATE	"XXXXXX"
#define	BACKUP_SUFFIX		".bak"

// the only safe way to use the tmp directory is to directly use /tmp
// Although POSIX says the env var TMPDIR is available, in practice, on Linux,
// this is not guaranteed. For example, Ubuntu does not have this variable.
// In any case, /tmp is also explicitly specified in POSIX.
#define	TEMPLATE_FOR_TMPDIR	"/tmp/XXXXXX"

#if defined(WIN32) || defined(UNIX)
static inline int safely_add_size_t_operands( size_t * result, size_t first, size_t second );

// note that we assume that filename != NULL, mkstemp_templatep != NULL but backup_filenamep may be NULL
int make_filenames( char * filename, char ** mkstemp_templatep, char ** backup_filenamep,
	int * use_copy_and_convert_methodp )
{
	size_t len_of_path_prefix ;
	size_t len ;
	char * s ;
	char * mkst_template ;
	char * backup_fname ;
	int	retval ;
	int dir_not_writeable ;
#if defined(UNIX)
	char first_char ;
#endif


#if !defined(UNIX)
	UNUSED_VARIABLE( use_copy_and_convert_methodp );
	dir_not_writeable = 0 ; // will always be 0 on Windows; used to avoid too convoluted #ifdefs later
							// hopefully, it will be optimized away by the compiler
#endif
	do {
		retval = -1 ;

		// calculate the size we need to allocate for the mkstemp() template
		s = strip_path_prefix( filename );	// s now points to the filename proper, sans path prefix
		if (s == filename) { // no path prefix
			len_of_path_prefix = 0 ;
			len = sizeof(MKSTEMP_TEMPLATE) ;
		}
		else if (*s == '\0') {
			// we had a a path prefix but no filename
			emsg( EMSG_BADFILENAME, filename );
			break ;
		}
		else { // else we have the normal path prefix and s now points to the filename
			len_of_path_prefix = s - filename ; // calculate length of path prefix; it will be > 0
			if (safely_add_size_t_operands( &len, len_of_path_prefix, sizeof(MKSTEMP_TEMPLATE) )) {
				emsg( EMSG_PATHTOOLONG, filename );
				break ;
			}
		}
#if defined(UNIX)
		// Check if directory is writeable; note that there's no point doing this for Windows,
		// since on Windows, access() always returns success for reading/writing directories.
		if (len_of_path_prefix) {
			first_char = *s ;
			*s = '\0' ;
			dir_not_writeable = access( filename, W_OK ) ;
			*s = first_char ;
		}
		else {
			dir_not_writeable = access( ".", W_OK );
		}
		if (dir_not_writeable) {
			if (!overwrite) {
				emsg( EMSG_DIRNOTWRITEABLE, filename );
				break ;
			}
			*use_copy_and_convert_methodp = 1 ;
			len = sizeof( TEMPLATE_FOR_TMPDIR );
			len_of_path_prefix = 0 ; // defensive (future-proof)
		}
#endif

		mkst_template = xmalloc( len );

#if defined(UNIX)
		if (dir_not_writeable) {
			strcpy( mkst_template, TEMPLATE_FOR_TMPDIR );
		}
#endif

		// written this way so that I don't have to use a hard-to-read #if defined(UNIX)
		// block with a dangling "else" that is not present in Windows compilation.
		// This way, hopefully on Windows, the compiler will just optimize away the
		// dir_not_writeable, since it's always 0 there, and the code can be common to both
		// with the same level of indentation.
		if (!dir_not_writeable) {
			if (len_of_path_prefix) {
				// if there is a path prefix, copy it into our newly allocated space
				strncpy( mkst_template, filename, len_of_path_prefix );
			}
			mkst_template[ len_of_path_prefix ] = '\0' ; // make sure it's null terminated (or a null string)
			strcat( mkst_template, MKSTEMP_TEMPLATE ); // affix the mkstemp() template, "XXXXXX".
		}

		*mkstemp_templatep = mkst_template ;

		// generate the backup filename if needed
		if (backup_filenamep != NULL) {
			// the backup filename is just the original filename + ".bak"
			// calculate the space we need to allocate
			if (safely_add_size_t_operands( &len, strlen( filename ), sizeof(BACKUP_SUFFIX) )) {
				emsg( EMSG_PATHTOOLONG, filename );
				break ;
			}
			backup_fname = xmalloc( len );
			strcpy( backup_fname, filename );
			strcat( backup_fname, BACKUP_SUFFIX );
			*backup_filenamep = backup_fname ;
		}

		retval = 0 ;

	} while (0) ;

	return retval ;
}

static inline int safely_add_size_t_operands( size_t * result, size_t first, size_t second )
{
	if ((SIZE_MAX - first) >= second) {
		*result = first + second ;
		return 0 ;
	}
	else
		return -1 ;
}

#elif defined(MSDOS)

#if !defined(_MAX_DIR) || (_MAX_DIR < 260)	/* MAXDIRSIZE */
#define MAXDIRSIZE	260
#else
#define	MAXDIRSIZE	_MAX_DIR
#endif
#if !defined(_MAX_NAME) || (_MAX_NAME < 260)	/* MAXFILESIZE */
#define MAXFILESIZE	260
#else
#define	MAXFILESIZE	_MAX_NAME
#endif
#if !defined(_MAX_PATH) || (_MAX_PATH < 260)	/* MAXPATHSIZE */
#define	MAXPATHSIZE	260
#else
#define	MAXPATHSIZE	_MAX_PATH
#endif
#if !defined(_MAX_DRIVE)
#define	_MAX_DRIVE	3		/* for the benefit of djgpp */
#endif

// note that we assume that filename != NULL, mkstemp_templatep != NULL but backup_filenamep may be NULL
int make_filenames( char * filename, char ** mkstemp_templatep, char ** backup_filenamep,
	int * use_copy_and_convert_methodp )
{
	char drv[_MAX_DRIVE] ;
	char dir[MAXDIRSIZE] ;
	char fname[MAXFILESIZE] ;
	char temp_filename[MAXPATHSIZE] ;

	UNUSED_VARIABLE( use_copy_and_convert_methodp );

	_splitpath( filename, drv, dir, fname, NULL );
	_makepath( temp_filename, drv, dir, MKSTEMP_TEMPLATE, NULL );
	*mkstemp_templatep = xstrdup( temp_filename );
	if (backup_filenamep != NULL) {
		_makepath( temp_filename, drv, dir, fname, BACKUP_SUFFIX );
		*backup_filenamep = xstrdup( temp_filename );
	}
	return 0 ;
}

#endif
