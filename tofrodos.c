/*
	tofrodos.c	Converts text files between DOS and Unix formats.
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

/* this should always be first */
#include "config.h"

/* standard headers */
#include <errno.h>	/* errno */
#include <stdio.h>	// fileno() (Windows/MSDOS)
#include <stdlib.h>	/* EXIT_SUCCESS */

/* conditionally included headers */
#if defined(MSDOS) || defined(WIN32)
#include <fcntl.h>	/* O_BINARY */
#include <io.h>		/* _setmode(), isatty() */
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	/* optind, isatty() */
#endif

#if defined(USE_SUPPLIED_GETOPT)
#include "getopt.h"	// optind
#else
#include <getopt.h>	// optind
#endif

/* our headers */
#include "emsg.h"
#include "tofrodos.h"
#include "utility.h"
#include "version.h"

/* global variables */
int abortonerr ;	/* 1 if should abort when there is error in any file */
					/* in a list of files, 0 carry on (default) */
conversion_direction_t direction = DIRECTION_UNSET ;
int forcewrite ; /* convert even if file is not writeable */
char * errorlogfilename ;	/* name of error log file, NULL if we're printing to stderr */
int preserve ;	/* 1 if we are to preserve owner (Unix) and date (all) */
char * progname = VERSN_PROGNAME ;/* name of binary (ie, argv[0] without a path prefix) */
int overwrite = 1 ;	/* 1 = overwrite original file, 0 = make backup */
int verbose ;
char * current_input_filename ; // for error messages


/*
	main

	tofrodos converts ASCII text files to/from a DOS CR-LF deliminated
	form from/to a Unix LF deliminated form.

	Usage: tofrodos [options] [file...]

	Exit codes:
		EXIT_SUCCESS	success	(stdlib.h)
		EXIT_ERROR		error	(tofrodos.h)
*/
int main ( int argc, char ** argv )
{
	int err ;
	int parse_retval ;

	/* initialise and parse the options */
	if (init( argv[0] ))
		return EXIT_ERROR ;
	parse_retval = parseargs( argc, argv );
	if (parse_retval == -1) {
		return EXIT_ERROR ;
	}
	else if (parse_retval == 1) {
		// user merely requested help or version info
		// and it has already been displayed
		return EXIT_SUCCESS ;
	}
	// else parse_retval == 0, and we can continue

	// make sure we know which direction we are supposed to convert
	if (direction == DIRECTION_UNSET) {
		emsg( EMSG_NODIRECTION );
		return EXIT_ERROR ;
	}

	/* check if we are to convert from stdin */
	if (argc == optind) {
	    if (isatty( fileno( stdin ) )) {
		/* stdin must be redirected else you should supply a */
		/* filename. */
		emsg( EMSG_NOFILENAME );
		return EXIT_ERROR ;
	    }
	    /* otherwise stdin has been redirected */
#if defined(MSDOS) || defined(WIN32)
		// need to make sure the input and output files are binary on MSDOS and Windows
	    _setmode( fileno( stdin ), O_BINARY );
	    _setmode( fileno( stdout ), O_BINARY );
#endif
	    return process_file( NULL ) ? EXIT_ERROR : EXIT_SUCCESS ;
	}

	/* if we reach here, we have a (list?) of files to convert */
	err = 0 ;
	while (optind < argc) {
	    if (verbose)
			emsg( VERBOSE_CONVERTING, argv[optind] );
	    if (((err = resolve_filename_and_convert( argv[optind] )) != 0) && abortonerr)
			return EXIT_ERROR ;
	    optind++ ;
	}

    return err ? EXIT_ERROR : EXIT_SUCCESS ;
}
