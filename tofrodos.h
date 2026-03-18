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

/* global variables */
extern int abortonerr ; /* 1 = abort list of files if error in any */
extern int alwaysconvert ;	// relic of an earlier version that is accessed in lots of places
							// (don't delete unless you root out all the references and re-test everything)
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
extern void make_filenames ( char * filename, char ** mkstemp_templatep, char ** backup_filenamep );
extern int parseargs ( int argc, char ** argv );
extern int process_file ( char * filename );
extern int resolve_filename_and_convert ( char * filename );

#if defined(__cplusplus)
}
#endif

#endif
