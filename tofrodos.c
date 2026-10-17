/*
	tofrodos.c	Converts text files between DOS and Unix formats.
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

/* this should always be first */
#include "config.h"

// standard headers
#include <errno.h>	//errno 
#include <stdio.h>	// fileno() (Windows/MSDOS)
#include <stdlib.h>	// EXIT_SUCCESS

// conditionally included headers
#if defined(MSDOS) || defined(WIN32)
#include <fcntl.h>	// O_BINARY
#include <io.h>		// _setmode(), isatty()
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>	// optind, isatty()
#endif

#if defined(USE_SUPPLIED_GETOPT)
#include "getopt.h"	// optind
#else
#include <getopt.h>	// optind
#endif

// our headers
#include "emsg.h"
#include "tofrodos.h"
#include "utility.h"
#include "version.h"

// global variables
int abortonerr ; // 1 = abort when there is error in any file; 0 skip to next file (default)
char * current_input_filename ; // for error messages
conversion_direction_t direction = DIRECTION_UNSET ;
sorted_array_t dir_excludes ;
sorted_array_t extensions ;
sorted_array_t file_excludes ;
int forcewrite ; // convert even if file is not writeable 
char * errorlogfilename ;	// name of error log file, NULL if we're printing to stderr
int preserve ;	// 1 if we are to preserve owner (Unix) and date (all)
char * progname = VERSN_PROGNAME ;
int overwrite = 1 ;	// 1 = overwrite original file; 0 = make backup
int recursive_mode ; // 0 = convert files on command line; 1 = recurse through directories
int verbose ;


/*
	main

	tofrodos converts ASCII text files to/from a DOS CR-LF delimited
	form from/to a Unix LF delimited form.

	Usage: tofrodos [options] [file...]

	Exit codes:
		EXIT_SUCCESS	success	(stdlib.h)
		EXIT_ERROR		error	(tofrodos.h)
*/
int main ( int argc, char ** argv )
{
	int err ;
	parseargs_retval_t parse_retval ;
	int exit_code ;

	do {
		exit_code = EXIT_ERROR ;

		/* initialise and parse the options */
		if (init( argv[0] )) {
			break ;
		}
		parse_retval = parseargs( argc, argv );
		if (parse_retval == PARSEARGS_EXIT_WITH_FAILURE) {
			break ;
		}
		else if (parse_retval == PARSEARGS_EXIT_WITH_SUCCESS) {
			// user merely requested help or version info
			// and it has already been displayed
			exit_code = EXIT_SUCCESS ;
			break ;
		}
		// else parse_retval == PARSEARGS_SUCCESS, and we can continue

		// make sure we know which direction we are supposed to convert
		if (direction == DIRECTION_UNSET) {
			emsg( EMSG_NODIRECTION );
			break ;
		}
		if (!recursive_mode && ((extensions.max != 0) || (dir_excludes.max != 0) || (file_excludes.max != 0))) {
			emsg( EMSG_NOT_RECURSIVE_MODE );
			break ;
		}

		if (recursive_mode) {
			// -i is required for recursive mode (else we have nothing to convert)
			if (extensions.max == 0) {
				emsg( EMSG_EXTREQUIRED );
				break ;
			}
			if (!recursively_process( argc, argv )) {
				exit_code = EXIT_SUCCESS ;
			}
		}
		else if (argc == optind) {
			/* check if we are to convert from stdin */
		    if (isatty( fileno( stdin ) )) {
				/* stdin must be redirected else you should supply a */
				/* filename. */
				emsg( EMSG_NOFILENAME );
				break ;
		    }
		    /* otherwise stdin has been redirected */
#if defined(MSDOS) || defined(WIN32)
			// need to make sure the input and output files are binary on MSDOS and Windows
		    _setmode( fileno( stdin ), O_BINARY );
		    _setmode( fileno( stdout ), O_BINARY );
#endif
			if (!process_file( NULL )) {
				exit_code = EXIT_SUCCESS ;
			}
		}
		else {
			err = 0 ;
			while (optind < argc) {
			    if (verbose) {
					emsg( VERBOSE_CONVERTING, argv[optind] );
				}
			    if (((err = resolve_filename_and_convert( argv[optind] )) != 0) && abortonerr) {
					break ;
				}
			    optind++ ;
			}
			if (!err) {
				exit_code = EXIT_SUCCESS ;
			}
		}
	} while (0) ;

	return exit_code ;
}
