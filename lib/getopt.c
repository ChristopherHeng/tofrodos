/*
	getopt.c
	Copyright 2026 by Christopher Heng. All rights reserved.

	This code is released under the terms of the GNU General Public
	License Version 3. You should have received a copy of the GNU
	General Public License along with this program; if not, write to the
	Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
	USA.

	Last updated: 10 April 2026:
	- disable warning from clang about cast removing const
	- added cast to size_t
*/
/*
	int getopt_long ( int argc, char * const * argv, const char * optstring,
		const struct option * longopts, int * longindex );
	int getopt( int argc, char * const * argv, const char * optstring );

	These functions implement getopt_long() and getopt() as found in Unix-type
	systems. They were written so that I can use them in my Windows programs.
	The differences between this and the GNU and BSD versions are described below.

	Missing Features
	----------------
	* The GNU extension "-W foo" (which it treats as equivalent to --foo), normally
	specified in optstring as "W;" (yes, with a semi-colon, not a colon), is not
	supported. My version does not treat 'W' differently from any other character.
	I don't think BSD's getopt() supports this either, though their getopt_long()
	probably does. For long options (of the "--foo" variety) in your programs,
	just use getopt_long() in the usual way instead of this clumsy construction.
	That's its reason for existence, after all.

	* The POSIXLY_CORRECT environment variable is completely ignored. If you want its
	functionality, use a '+' as the first character in optstring. The latter is compatible
	with the GNU and BSD versions of these functions as well.

	* The _<PID>_GNU_nonoption_argv_flags_ environment variable is completely ignored.
	This is used in the GNU version to support a long obsolete version of Bash.

	Other Implementation Differences
	--------------------------------

	* The has_arg member of struct option is an enum in my implementation, and not an int.
	The values for this member (ie, no_argument, required_argument, and optional_argument)
	are also enums, and not macros. It's done this way so that it's easier to catch errors,
	since the compiler can then check that you have assigned has_arg a valid value.
	The GNU and BSD implementations have has_arg as an int. The values for it may either
	be macros (GNU and possibly BSD) or enums (possibly MinGW-w64, though I'm not sure).
	That said, if you just include "getopt.h", and don't manually declare either the structure
	or the various names of the has_arg values, your code should compile without any issue,
	even if you have coded it for a different getopt*() implementation.

	* Like the GNU and POSIX versions, my getopt() returns '?' for missing option arguments
	when optstring does not have an initial ":". The MinGW-w64/BSD version, however, returns
	':' for historical reasons, whether or not you use ":" in optstring. If you want the
	BSD behaviour, put an initial ":" in optstring, which will work with all implementations.

	* This version does not allow ':' and '-' as short option characters. The BSD version
	apparently allows '-' for backward compatibility reasons. I'm not sure what it does
	with ':'. Use of either of these as options on the command line will cause my
	functions to reject it as an invalid option, even if you put them in optstring.

	* Differences in the way the options are parsed show in the behaviour of getopt_long()
	when the user concatenates short options and embeds '-' in the midst of those options.
	Take for example the following command line with a block of concatenated short options:

		programname -a-b

	In my version, getopt_long() sees 3 options: "-a", "--" (which will be reported
	as invalid, since it doesn't allow '-' as an option character), and "-b".
	The GNU version does likewise. MinGW-w64/BSD's getopt_long(), however, sees
	2 options: "-a" and "--b".

	I don't think the MinGW-w64/BSD version is correct here, since concatenation is
	meant to be for short options, and should not be an underhanded way to slip in a
	long option by inserting an additional '-' midstream. Note that this difference
	only applies to getopt_long(). The behaviour of getopt() is identical in both
	the BSD and my versions, since there are no long options there.

	* My getopt() follows the GNU default behaviour of shifting the arguments so
	that those with options precede those without. This is not the case with
	the MinGW-w64/BSD version, which follows POSIX in terminating the series of
	options when a non-option argument is encountered. If you want your code to
	behave identically with the MinGW-w64/BSD version, put "+" as the first
	character in the optstring, which will cause both my and the GNU versions
	to leave the arguments unsorted. The "+" is ignored by the BSD getopt(),
	since it is already behaving as though it was specified, so your code will work
	fine there too. Note that I am talking about getopt() here, not getopt_long().
	All versions of getopt_long() shift arguments by default.

	* If you want to reuse getopt_long() and getopt() after processing the command
	line, you may either use the BSD method of setting optreset to 1, or the
	GNU method of setting optind to 0. If you use the BSD method, you must
	also set optind to point to the arg that you want the functions to start
	processing from. The GNU method will always cause them to start
	processing from 1 (the second element in argv). Note that regardless of
	method, you should point argv[0] to your program name (the way main()'s
	argv does), since the functions use argv[0] in error messages.

	* When flag != NULL in struct option, the integer pointed to will be set
	to val as expected. This implementation also sets optopt to that same value.
	This is apparently the case with the GNU version of getopt_long() as well
	(unverified), though not the BSD version.

	* When a long option has a missing required argument, or it
	has an argument when it wasn't supposed to, this implementation
	sets *longindex to indicate the long option that triggered the error. The
	MinGW-w64/BSD version leaves *longindex untouched. In fact, since it
	also does not set optopt to the option that caused the error, you have
	no way to figure out which opt caused the problem, except to access
	argv[optind-1] and parse it yourself. In my implementation, both optopt
	and longindex indicate the problematic option. That said, if you need
	something compatible with all implementations so that you can print
	relevant error messages, either check argv[optind-1] or just allow
	getopt_long() to print its own error message.
*/

#if defined(_MSC_VER)
// fprintf() function not inlined
#pragma warning(disable: 4710)
// ompiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable: 5045)
// function 'getopt_long()' selected for automatic inline expansion
#pragma warning(disable: 4711)
#endif

#if defined(__clang__)
// disable warnings about unsafe buffer access and unsafe pointer arithmetic
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
// disable warnings about cast from "char * const *" to "char **" dropping const qualifier
#pragma clang diagnostic ignored "-Wcast-qual"
#endif

#include <stdio.h>	// fprintf()
#include <string.h> // strncmp(), strchr(), strlen(), size_t
#include "getopt.h"

// macros
#define	RETVAL_OPT_IN_FLAG		0	// option returned in *(option->flag)
#define	RETVAL_ARG_IN_OPTARG	1
#define	RETVAL_NO_MORE_OPTIONS	-1
#define	RETVAL_ERR				((int) '?')
#define	RETVAL_MISSING_OPTARG	((int) ':')

// enums
typedef enum {
	shift_arguments_if_needed = 0,
	end_opts_on_normal_arguments = 1,
	arguments_are_optargs = 2
} arg_disposition_t ;

// global variables
char * optarg ;		// argument to option
int opterr = 1 ;	// 0 = don't print error msg, 1 = print
int optind = 1 ;	// next argument to parse
int optopt ;		// the current option
int optreset ;		// 0 = continue processing, 1 = reset to start

// local variables
static char empty_string[] = "" ; // never changed (effectively a const)
static int missing_argument_retval ;
static arg_disposition_t arg_disposition ;
static char * curptr = empty_string ; // next option character in an arg with concatenated short options

// for tracking the shifting of args up the array
static int shift_in_progress ;
static int saved_optind ; // actual current argv item
static int tracked_optind ; // real final optind
static int num_of_args_to_shift ;
static int num_of_args_for_opt ;
static int dest_end_index ;
static char * saved_arg_with_opt ;
static char * saved_arg_with_optarg ;

// local functions
static inline int get_index_of_matching_longopt( const struct option * longopts, size_t len_of_opt_name );
static inline int look_for_arg_with_opt( int argc, char * const * argv );
static inline void point_to_next_arg( void );
static int process_long_opt( int argc, char * const * argv, const struct option * longopts, int * longindex );
static inline int process_short_opt( int argc, char * const * argv, const char * optstring );
static inline void shift_args( char * const * argv );


int getopt( int argc, char * const * argv, const char * optstring )
{
	return getopt_long( argc, argv, optstring, NULL, NULL );
}

int getopt_long ( int argc, char * const * argv, const char * optstring,
	const struct option * longopts, int * longindex )
{
	int found_an_opt ;
	int retval ;

	if (optreset || (optind == 0)) { // asked to reset
		// optreset=1 is BSD's method; optind=0 is GNU's.
		optreset = 0  ;
		if (optind == 0) {
			// WARNING: only change optind if it's zero, since that's an invalid value that
			// points to argv[0], the program name. Otherwise, leave it alone so that the
			// user can control where we start processing.
			optind = 1 ;
		}
		curptr = empty_string ;
	}

	// The following is done on every call, even though theoretically, we only
	// have to do it once until optreset. But we always have to increment
	// optstring past the initial '+'/'-' and ':' so that when we
	// do a strchr() later, we won't match these (especially the '+', which
	// we don't specifically disallow as an opt). Since we're doing
	// that anyway, we might as well just do the other stuff, for
	// the sake of simplifying the code.
	arg_disposition = shift_arguments_if_needed ; // defaults to moving options before normal args
	if (*optstring == '+') { // caller wants the POSIX behaviour of not shifting
		arg_disposition = end_opts_on_normal_arguments ;
		optstring++ ;
	}
	else if (*optstring == '-') { // return normal arguments as optarg to option 1
		arg_disposition = arguments_are_optargs ;
		optstring++ ;
	}
	if (*optstring == ':') {
		// initial ':' means we suppress error messages and return a ':' instead of '?' when
		// an option is missing a required argument.
		missing_argument_retval = RETVAL_MISSING_OPTARG ;
		opterr = 0 ;
		optstring++ ;
	}
	else {
		// else we don't touch opterr and the default return value is '?'
		missing_argument_retval = RETVAL_ERR ;
	}

	do {

		optarg = NULL ;

		shift_in_progress = 0 ;
		saved_optind = -1 ;
		tracked_optind = -1 ;
		num_of_args_to_shift = 0 ;
		num_of_args_for_opt = 0 ;
		dest_end_index = 0 ;
		saved_arg_with_opt = NULL ;
		saved_arg_with_optarg = NULL ;

		if (*curptr == '\0') { // need to start on the next arg (or the 1st, if this is our first run)
			if (optind < argc) { // still more to go
				curptr = argv[optind] ;
			}
			else { // we've run out of args
				optopt = 0 ;
				curptr = empty_string ; // should already be pointed
				retval = RETVAL_NO_MORE_OPTIONS ;
				break ;
			}
		}
		else {
			// if curptr doesn't point to a null byte, it means we're in the middle of processing
			// an arg containing concatenated short options
			retval = process_short_opt( argc, argv, optstring );
			break ;
		}

		if ((*curptr == '-') && (*(curptr+1) != '\0')) { // not a solitary '-' which is treated as a non-opt arg
			found_an_opt = 1 ;
		}
		else { // normal (non-option) argument encountered
			if (arg_disposition == shift_arguments_if_needed) {
				found_an_opt = (look_for_arg_with_opt( argc, argv ) == 0) ? 1 : 0 ;
			}
			else if (arg_disposition == arguments_are_optargs) {
				// all normal arguments are treated as arguments to option 1
				optopt = RETVAL_ARG_IN_OPTARG ;
				optarg = curptr ;
				point_to_next_arg();
				retval = RETVAL_ARG_IN_OPTARG ;
				break ;
			}
			else { // not allowed to shift; therefore we have finished with the options
				found_an_opt = 0 ;
			}
		}

		if (!found_an_opt) {
			optopt = 0 ;
			curptr = empty_string ;
			retval = RETVAL_NO_MORE_OPTIONS ;
			break ;
		}

		// at this point *curptr == '-'
		if ((*(curptr+1) == '-') && (*(curptr+2) == '\0')) { // skip ahead to check for "--"
			// we have reached the end of the options
			point_to_next_arg();
			optopt = 0 ;
			retval = RETVAL_NO_MORE_OPTIONS ;
		}
		else if (longopts == NULL) { // we're running in getopt() mode
			// point *curptr to option char
			curptr++ ;
			retval = process_short_opt( argc, argv, optstring );
		}
		else { // we're running in getopt_long() mode
			curptr++ ;
			if (*curptr == '-') { // long opt
				retval = process_long_opt( argc, argv, longopts, longindex );
			}
			else { // short opt
				retval = process_short_opt( argc, argv, optstring );
			}
		}

		if (shift_in_progress) {
			// need to actually shift the arguments and set optind to the correct value
			shift_args( argv );
			((char **)argv)[saved_optind] = saved_arg_with_opt ;
			if (num_of_args_for_opt > 1) {
				((char **)argv)[saved_optind+1] = saved_arg_with_optarg ;
			}
			optind = tracked_optind ;
		}

	} while (0);

	return retval ;
}

static inline void point_to_next_arg( void )
{
	optind++ ;
	if (shift_in_progress) {
		tracked_optind++ ;
	}
	curptr = empty_string ;
}

static inline int look_for_arg_with_opt( int argc, char * const * argv )
{
	int retval ;

	saved_optind = optind ;
	tracked_optind = optind ;

	do {
		optind++ ;
		num_of_args_to_shift++ ;
		if (optind < argc) {
			curptr = argv[optind] ;
		}
		else {
			curptr = empty_string ;
			break ;
		}
	} while ((*curptr != '-') || ((*curptr == '-') && (*(curptr+1) == '\0')));

	if ((*curptr != '-') || ((*curptr == '-') && (*(curptr+1) == '\0'))) {
		// we have run out of argv without finding an opt
		optind = saved_optind ;
		retval = -1 ;
	}
	else {
		num_of_args_for_opt = 1 ;
		dest_end_index = optind ;
		saved_arg_with_opt = curptr ;
		shift_in_progress = 1 ;
		retval = 0 ;
	}

	return retval ;
}


static inline void shift_args( char * const * argv )
{
	int i ;
	int j ;

	j = dest_end_index ;
	for (i = saved_optind + num_of_args_to_shift - 1; i >= saved_optind; i--, j--) {
		((char **)argv)[j] = argv[i] ;
	}
}

static inline int process_short_opt( int argc, char * const * argv, const char * optstring )
{
	char * s ;
	int retval ;

	// on entry, *curptr points to opt
	optopt = *curptr ;
	if ((optopt == ':') || (optopt == '-')) {
		// Unlike the BSD getopt family, we don't allow '-' as an opt character.
		// Note that although the calling function would have ruled out an outright "--", it could
		// still appear in the middle of an arg containing concatenated options like "-cx-v"
		if (opterr) {
			fprintf( stderr, "%s: invalid option -%c\n", argv[0], optopt );
		}
		curptr++ ;
		retval = RETVAL_ERR ;
	}
	else {

		s = strchr( optstring, optopt );
		if (s == NULL) {
			if (opterr) {
				fprintf( stderr, "%s: invalid option -%c\n", argv[0], optopt );
			}
			curptr++ ;
			if (*curptr == '\0') {
				point_to_next_arg();
			}
			retval = RETVAL_ERR ;
		}
		else {
			s++ ;
			if (*s == ':') {
				s++ ;
				if (*s == ':') { // optional arg
					// for optional arguments to short options, the argument must be in the same arg
					curptr++ ;
					if (*curptr != '\0') {
						optarg = curptr ;
					}
					// else optarg is already NULL (we zeroed it at the beginning of getopt_long()
					point_to_next_arg();
					retval = optopt ;
				}
				else { // required arg
					curptr++ ;
					if (*curptr == '\0') {
						point_to_next_arg();
						if (optind < argc) {
							optarg = argv[optind] ;
							if (shift_in_progress) {
								num_of_args_for_opt++ ;
								dest_end_index = optind ;
								saved_arg_with_optarg = optarg ;
							}
							point_to_next_arg();
							retval = optopt ;
						}
						else {
							if (opterr) {
								fprintf( stderr, "%s: option -%c requires an argument.\n", argv[0], optopt );
							}
							retval = missing_argument_retval ;
						}
					}
					else { // required optarg is in the same arg
						optarg = curptr ; // this must be done first (before we reset curptr)
						point_to_next_arg();
						retval = optopt ;
					}
				}
			} // if (*s == ':')
			else { // simple option with no arg
				curptr++ ;
				if (*curptr == '\0') {
					point_to_next_arg();
				}
				retval = optopt ;
			}
		}
	}
	return retval ;
}

static int process_long_opt( int argc, char * const * argv, const struct option * longopts, int * longindex )
{
	char * arg ;
	char * temp_long_opt_str ;
	int i ;
	int retval ;
	size_t len_of_opt_name ;

	// on entry, *curptr == '-' (the second of "--") and hence a long opt
	curptr++ ;
	// *curptr != '\0' because we have already checked before calling process_long_opt()

	// long options may be of the form --longopt=argument or --longopt argument or just --longopt
	// need to separate the long option from its argument if any
	len_of_opt_name = strlen( curptr );
	arg = strchr( curptr, (int) '=' );
	if (arg != NULL) {
		// adjust the option name length to account for the fact that it has an argument
		len_of_opt_name = (size_t) (arg - curptr) ;
		arg++ ; // skip past '='
	}

	do {
		i = get_index_of_matching_longopt( longopts, len_of_opt_name );
		temp_long_opt_str = curptr ; // save it for error messages if needed
		if (i >= 0) {
			if (longindex != NULL) {
				*longindex = i ;
			}
			if ((longopts[i]).has_arg == required_argument) {
				if (arg != NULL) {
					// argument followed an '='
					optarg = arg ;
					// we're done with this argv
					point_to_next_arg();
					optopt = (longopts[i]).val ;
					if ((longopts[i]).flag == NULL) {
						retval = optopt ;
					}
					else {
						*((longopts[i]).flag) = optopt ;
						retval = RETVAL_OPT_IN_FLAG ;
					}
					break ;
				}
				// the optarg is the next argument
				point_to_next_arg();
				if (optind < argc) {
					optarg = argv[optind] ;
					if (shift_in_progress) {
						saved_arg_with_optarg = optarg ;
						num_of_args_for_opt++ ;
						dest_end_index = optind ;
					}
					point_to_next_arg();

					optopt = (longopts[i]).val ;
					if ((longopts[i]).flag == NULL) {
						retval = optopt ;
					}
					else {
						*((longopts[i]).flag) = optopt ;
						retval = RETVAL_OPT_IN_FLAG ;
					}
					break ;
				}
				// else it's a case of missing opt argument
				if (opterr) {
					fprintf( stderr, "%s: option --%s requires an argument.\n", argv[0], temp_long_opt_str );
				}
				optopt = (longopts[i]).val ;
				retval = missing_argument_retval ;
				break ;
			}
			if ((longopts[i]).has_arg == optional_argument) {
				// optional argument, if present, must be in the same arg as the option
				if (arg != NULL) {
					optarg = arg ;
				}
				// else optarg is already NULL; it was init at the beginning of getopt_long()
				optopt = (longopts[i]).val ;
				if ((longopts[i]).flag == NULL) {
					retval = optopt ;
				}
				else {
					*((longopts[i]).flag) = optopt ;
					retval = RETVAL_OPT_IN_FLAG ;
				}
				// we're done with this argv
				point_to_next_arg();
				break ;
			}
			// else it means ((longopts[i]).has_arg == no_argument)
			if (arg != NULL) { // error - it has an argument after all
				if (opterr) {
					fprintf( stderr, "%s: unexpected argument to option --%s\n", argv[0], temp_long_opt_str );
				}
				optarg = arg ;
				optopt = (longopts[i]).val ;
				retval = missing_argument_retval ;
			}
			else {
				optopt = (longopts[i]).val ;
				if ((longopts[i]).flag == NULL) {
					retval = optopt ;
				}
				else {
					*((longopts[i]).flag) = optopt ;
					retval = RETVAL_OPT_IN_FLAG ;
				}
			}
			// regardless of whether there's an unwanted arg, we're done with this argv
			point_to_next_arg();
			break ;
		} // if (i >= 0)
		else if (i == -2) {
			if (opterr) {
				fprintf( stderr, "%s: option --%s is ambiguous\n", argv[0], curptr );
			}
			optopt = 0 ;
			point_to_next_arg();
			retval = RETVAL_ERR ;
			break ;
		}
		else { // i == -1
			if (opterr) {
				fprintf( stderr, "%s: invalid option --%s\n", argv[0], curptr );
			}
			optopt = 0 ;
			point_to_next_arg();
			retval = RETVAL_ERR ;
			break ;
		}
	} while (0);

	return retval ;
}

static inline int get_index_of_matching_longopt( const struct option * longopts, size_t len_of_opt_name )
{
	const struct option * opts_ptr ;
	int prev_partial_match ;
	int retval ;
	int i ;
	int found ;

	opts_ptr = longopts ;
	prev_partial_match = -1 ;
	found = 0 ;
	retval = -1 ; // not found

	for (i = 0; opts_ptr->name != NULL; i++, opts_ptr++) {
		if (!strncmp( opts_ptr->name, curptr, len_of_opt_name )) {
			// we matched the characters up to the length of curptr
			if (strlen( opts_ptr->name ) == len_of_opt_name) {
				// exact match
				retval = i ;
				found = 1 ;
				break ;
			}
			// else it's a partial match
			if (prev_partial_match == -1) {
				// this is the first partial match we've found
				prev_partial_match = i ;
			}
			else {
				// we previously found another partial match
				retval = -2 ; // option is ambiguous
				found = 1 ;
				break ;
			}
		}
	} // for (i = 0; opts_ptr->name != NULL; i++, opts_ptr++)
	if (!found && (prev_partial_match != -1)) {
		retval = prev_partial_match ;
	}
	return retval ;
}
