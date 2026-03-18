/*
	init.c		Initialisation functions.
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

/* this should always be first */
#include "config.h"

/* standard headers */
#include <stdlib.h>	/* _splitpath() (MSDOS), exit, EXIT_SUCCESS */
#include <stdio.h>	/* fprintf() */
#include <string.h>	/* strcasecmp(), strrchr() */

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(USE_SUPPLIED_GETOPT)
#include "getopt.h"
#else
#include <getopt.h>
#endif

/* our own headers */
#include "emsg.h"
#include "tofrodos.h"
#include "utility.h"
#include "version.h"

/* macros */
#define	HELPFMT		"\nUsage: %s [options] [file...]\n"\
					"-b, --backup\t\t\tBack up original file (.bak).\n"\
					"-d, --dos2unix, --fromdos\tConvert from DOS to Unix.\n"\
					"-e, --exit-on-error\t\tSkip remaining files on error.\n"\
					"-f, --force\t\t\tConvert even if file is read-only.\n"\
					"-h, --help\t\t\tDisplay help on usage and quit.\n"\
					"-l logfile, --log=logfile\tLog most errors and verbose messages to <logfile>\n"\
					"-o, --overwrite\t\t\tOverwrite original file (default).\n"\
					"-p, --preserve\t\t\tPreserve file owner and time.\n"\
					"-u, --unix2dos, --todos\t\tConvert from Unix to DOS.\n"\
					"-v, --verbose\t\t\tVerbose.\n"\
					"-V, --version\t\t\tShow version and quit.\n"
#define	VERFMT		"%s Ver %d.%d.%d "\
			"Converts text files between DOS and Unix formats.\n"\
			"Copyright 1996-2026 Christopher Heng. "\
                        "All rights reserved.\n"\
			"https://github.com/ChristopherHeng/tofrodos\n"

#if defined(MSDOS) || defined(WIN32)
#if !defined(_MAX_NAME) || (_MAX_NAME < 260)
#define MAXFILESIZE	260
#else
#define	MAXFILESIZE	_MAX_NAME
#endif
#endif


/* local functions */
static void showhelp ( void );
static void showversion ( void );

/*
	init

	Checks for correct operating system version (DOS only).
	Sets the default direction of conversion.

	Returns 0 on success, -1 on error.
*/
int init ( char * firstarg )
{
#if defined(MSDOS)
	char filename[MAXFILESIZE];
#elif defined(WIN32)
	int restore_dot ;
	char * s ;
#endif

#if defined(MSDOS)
	/* Check that we have the minimum version of DOS needed. */
	/* We only run on DOS 3.1 and above. */
	if (_osmajor < 3 ||
		(_osmajor == 3 && _osminor < 10)) {
		emsg( EMSG_WRONGDOSVER );
		return -1 ;
	}
#endif

	/* set the name of the binary to set default direction of */
	/* conversion */
#if defined(MSDOS)
	/* got to extract the name from the full path and extension */
	_splitpath( firstarg, NULL, NULL, filename, NULL );
	progname = xstrdup( filename );
#elif defined(WIN32)
	// note that although strip_path_prefix() can handle '/' in paths, Command Prompt does
	// not recognize it (at least, up to Windows 11). That said, we should still support it
	// in case this program is run from other shells that do.
	progname = strip_path_prefix( firstarg );
	restore_dot = 0 ;
	if ((s = strrchr( progname, '.' )) != NULL) {
		if (!strcasecmp( s, ".exe" )) {
			*s = '\0' ;
			restore_dot = 1 ;
		}
	}
	progname = xstrdup( progname );
	if (restore_dot) {
		*s = '.' ; // restore what we modified (just in case we need it in the future)
	}
#else	// UNIX
	progname = strip_path_prefix( firstarg );
#endif // else clause of defined(MSDOS)

	// set the default direction of conversion
	if (!strcasecmp( progname, "fromdos" ) ||
		!strcasecmp( progname, "dos2unix" ))
		direction = DIRECTION_DOS2UNIX ;
	else if (!strcasecmp( progname, "todos" ) ||
		!strcasecmp( progname, "unix2dos" ))
		direction = DIRECTION_UNIX2DOS ;

	return 0 ;
}

/*
	parseargs

	Parses the options.

        Returns:
        	1 if the user requested help or the version number, and the program should exit with success
        	0 if the options were successfully parsed, and processing should continue
        	-1 on error, and program should terminate
*/
int parseargs ( int argc, char ** argv )
{
	static char optstring[] = "bdefhl:opuvV" ;
	static struct option longopts[] = {
		{ "backup", no_argument, NULL, 'b' },
		{ "dos2unix", no_argument, NULL, 'd' },
		{ "fromdos", no_argument, NULL, 'd' },
		{ "exit-on-error", no_argument, NULL, 'e' },
		{ "force", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "log", required_argument, NULL, 'l' },
		{ "overwrite", no_argument, NULL, 'o' },
		{ "preserve", no_argument, NULL, 'p' },
		{ "unix2dos", no_argument, NULL, 'u' },
		{ "todos", no_argument, NULL, 'u' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "version", no_argument, NULL, 'V' },
		{ NULL, no_argument, NULL, 0 }
	} ;
	int c ;
	int retval ;
	char * orig_arg0 ;

	orig_arg0 = argv[0] ;
	argv[0] = progname ; // so that getopt() prints errors with the same name as our other error messages
	retval = 0 ; // assume success, and that the caller should continue
	while ((c = getopt_long( argc, argv, optstring, longopts, NULL )) != -1) {
		switch( c ) {
			case 'b': /* make backup of original file */
				overwrite = 0 ;
				break ;
			case 'd': /* DOS to Unix */
				direction = DIRECTION_DOS2UNIX ;
				break ;
			case 'e': /* abort processing list of files if */
				  /* we encounter errors in any file in */
				  /* a list of file names */
				abortonerr = 1 ;
				break ;
			case 'f': /* convert even if file is not writeable*/
				forcewrite = 1 ;
				break ;
			case 'h': /* display short usage screen and quit */
               	showhelp() ;
            	retval = 1 ; // program should terminate with EXIT_SUCCESS
				break ;
            case 'l': /* log errors to filename */
            	errorlogfilename = optarg ;
            	break ;
			case 'o': /* overwrite original file (default) */
            	overwrite = 1 ;
            	break ;
			case 'p': /* preserve file owner and date */
				preserve = 1 ;
				break ;
			case 'u': /* Unix to DOS */
				direction = DIRECTION_UNIX2DOS ;
				break ;
			case 'v': /* verbose */
				verbose = 1 ;
				break ;
			case 'V': /* show version and quit */
				showversion() ;
				retval = 1 ; // program should terminate with EXIT_SUCCESS
				break ;
			default:
				retval = -1 ; // error message has already been printed
				break ;
		}
	}
	argv[0] = orig_arg0 ; // currently redundant, but just in case (future-proof)
	return retval ;
}

static void showversion ( void )
{
	static int vershown ;

	if (!vershown) {
		fprintf( stderr, VERFMT, VERSN_PROGNAME, VERSN_MAJOR, VERSN_MINOR, VERSN_PATCH );
		vershown = 1 ;
	}
	return ;
}

/*
	showhelp

	Display the short usage help screen.
*/
static void showhelp ( void )
{
	showversion();
	fprintf( stderr, HELPFMT, progname );
	return ;
}
