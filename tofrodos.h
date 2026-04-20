/*
	tofrodos.h	Converts text files between DOS and Unix formats.
	Copyright (c) 1996-2026 by Christopher Heng. All rights reserved.
*/

#if !defined(TOFRODOS_H_INCLUDED)
#define	TOFRODOS_H_INCLUDED

#include <stdio.h>	// because convert_file() takes FILE * as arguments

#if defined(__cplusplus)
extern "C" {
#endif

// enums
typedef enum { DIRECTION_UNSET = 0, DIRECTION_UNIX2DOS = 1, DIRECTION_DOS2UNIX = 2 } conversion_direction_t ;

/* macros */
#define	EXIT_ERROR	1	/* exit code on error */

// cannot use [[maybe_unused]] since Visual Studio 2026 does not support it for C code (yet)
#define	UNUSED_VARIABLE(x)	((void)(x))

/* global variables */
extern int abortonerr ; /* 1 = abort list of files if error in any */
extern char * current_input_filename ; // current input filename (for error messages)
extern conversion_direction_t direction ;
extern int forcewrite ; /* convert even if file is not writeable */
extern char * errorlogfilename ; /* name of error log file, NULL if we're printing to stderr */
extern int overwrite ; /* 1 = overwrite (default), 0 = make backup */
extern int preserve ;	/* 1 if we are to preserve owner (Unix) and date (all) */
extern char * progname ; /* name of binary */
extern int verbose ; /* 1 = be noisy, 0 = shut up */

/* function prototypes */
extern int convert_file( FILE * infp, FILE * outfp );
extern int init ( char * firstarg );
extern int make_filenames( char * filename, char ** mkstemp_templatep, char ** backup_filenamep,
	int * use_copy_and_convert_methodp );
extern int parseargs ( int argc, char ** argv );
extern int process_file ( char * filename );
extern int resolve_filename_and_convert ( char * filename );

#if defined(DECLARE_CHECK_AND_SAVE_FILE_INFO)
	// we only declare this if needed, to avoid having to include utime.h, sys/types.h and sys/stat.h
extern int check_and_save_file_info ( char * filename, mode_t * origfilemodep,
	struct utimbuf * filetimebufp, uid_t * ownerp, gid_t * groupp,
	int * use_copy_and_convert_methodp, int * need_to_make_writeablep );
#endif

#if defined(__cplusplus)
}
#endif

#endif
