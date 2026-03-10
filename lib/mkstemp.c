/*
	mkstemp.c	Create a temporary file name.
	Copyright 1996-2026 by Christopher Heng. All rights reserved.

	This code is released under the terms of the GNU General Public
	License Version 2. You should have received a copy of the GNU
	General Public License along with this program; if not, write to the
	Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
	USA.

	int mkstemp ( char * templ );

	mkstemp() creates a temporary file using "templ" and returns a file descriptor opened using open().
	The file is open for read and write binary (not text) access for the current user. The new filename
	is placed in "templ", overwriting its existing contents.

	The file path in "templ" must have six trailing "X"s, ie, it must end with "XXXXXX".

	On success, mkstemp() returns the file descriptor.

	On failure, it returns -1, and errno is set to EINVAL if "templ" does not end with "XXXXXX" on
	entry to the function, or EEXIST if no file could be created. If there is some sort of file system
	error so that we cannot create the file, the errno will contain the code for that error.

	Implementation details:
	- This implementation replaces the "XXXXXX" with the pattern represented by the regex [0-9]{5}[a-z0-9_-].
	The function tries to create a filename with the last 5 digits of the current time (hence the [0-9]{5}),
	as returned by time(), followed by a final character from the class [a-z0-9_-]. Each character in the
	class is tried in turn. If a unique file cannot be created, the time is incremented. This continues
	until the the 5-digit number loops back to the original number, at which time the function gives up.
	In other words, it tries (99,999 * (26 + 10 + 2)) times in total.
	- Note that this implementation does not follow the OpenBSD and MinGW-w64's method of using more than 6 Xs
	if the caller provides it. It strictly uses the last 6 Xs, and that's it, just like the old Unix libc,
	GNU libc and presumably also POSIX versions.

	2 March 2026
	- bug fix: no longer opens file with O_BINARY
*/

#if defined(_MSC_VER)
// function 'function' selected for inline expansion (probably only needed if you compile with /Wall)
#pragma warning(disable: 4711)
// Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified (probably only with /Wall)
#pragma warning(disable: 5045)
// disable warnings about using open()
#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>	// static_assert(), requires C11 or later
#include <errno.h>	// errno, EINVAL, EEXIST
#include <fcntl.h>	// open(), O_CREAT, O_TRUNC, O_EXCL, O_BINARY, O_RDWR and their VS equivalents (prefixed with '_')
#include <limits.h>	// INT_MAX for use with static_assert()
#include <string.h>	// strlen()
#include <time.h>	// time(), time_t
#include <sys/stat.h>	// S_IRUSR and S_IWUSR (non-VS); or _S_IREAD and _S_IWRITE (VS)

#if defined(__MINGW32__)
#include <stdlib.h>	// mkstemp() is declared here in MinGW-w64 (as it is in POSIX)
#endif

#if defined(_MSC_VER)
#include <io.h>	// open() for VS
#define	S_IRUSR	S_IREAD
#define	S_IWUSR	S_IWRITE
#endif

// macros
#define MKTEMP_TEMPLATE 		"XXXXXX"
#define MAX_DIGITS_CONSTRAINT	10000

// local functions
static void make_into_string( char * target_string, unsigned long current_value, char first_char );

#if defined(__MING32__)
	/*
		For some reason, mkstemp() is marked as __cdecl in MinGW-w64 (at least up to version 10.0.0)
		even though there's no need for it.
	*/
int __cdecl mkstemp ( char * templ )
#else
int mkstemp ( char * templ )
#endif
{
	static const char extra_char_list[] = "abcdefghijklmnopqrstuvwxyz0123456789_-" ;
	size_t len ;
	char * start_of_x ;
	long initial_value ; // long because it's guaranteed by Std C to be at least 32 bits, and therefore > 99,999
	long current_value ;
	int i ;
	int	fd ;

	len = strlen( templ );
	if ((len < 6) ||
		strcmp( (start_of_x = templ + len - 6), MKTEMP_TEMPLATE )) {
		errno = EINVAL ;
		return -1 ;
	}

	initial_value = (long) ( time( NULL ) % MAX_DIGITS_CONSTRAINT ); // use last 5 digits of time_t from time()
	current_value = initial_value ;
	do {
		/*
		Note: the cast to (int) below for the constant (sizeof(extra_char_list)-1) is for clang -Wall -Wextra's benefit.
		I don't like it because it silences any compiler warning should I ever change extra_char_list[]
		to be larger than int, although this is unlikely (and ridiculous). That said, I added the
		static_assert() below to make sure that it doesn't happen.
		*/
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
		// Note that for Visual Studio, you must compile this source file with /std:c11 or /std:c17 or
		// it will not define __STDC_VERSION__. Info from
		// https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170
		static_assert( (sizeof(extra_char_list) <= INT_MAX), "extra_char_list[] must not be bigger than INT_MAX" );
#endif

		for (i = 0; i < (int) (sizeof(extra_char_list) - 1); i++) {
			make_into_string( start_of_x, current_value, extra_char_list[i] );
			fd = open( templ, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR );
			if (fd != -1) {
				return fd ;
			}
			else if (errno != EEXIST) {
				return -1 ;
			}
		}
		// increment the value, but keep only the last 5 digits (so that we can compare for the loop)
		current_value = (current_value + 1) % MAX_DIGITS_CONSTRAINT ;
	} while (current_value != initial_value);
	// errno should already be EEXIST
	return -1 ;
}

static void make_into_string( char * target_string, unsigned long current_value, char extra_char )
{
	int i ;

	target_string[5] = extra_char ;
	for (i = 4 ; i >= 0 ; i--) {
		target_string[i] = (char) ((current_value % 10) + '0'); // extract the final digit and convert to ASCII
		current_value /= 10 ; // shift decimal digits right by one
	}
}
