/*
	set_test_filetime.c
	Copyright 2026 Christopher Heng. All rights reserved.

	Sets the accessed and modified time of a file to 1st January 2013 6am,
	so that we can test the -p option of tofrodos. For Windows only. On
	other systems, just use "touch".
*/

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif

static inline int set_test_filetime( char * filename );

int main ( int argc, char ** argv )
{
	if (argc != 2) {
		fprintf( stderr, "%s: Usage: %s filename\n", argv[0], argv[0] );
		return 1 ;
	}
	if (set_test_filetime( argv[1] ) == -1) {
		fprintf( stderr, "%s: Unable to set file time of %s to 1st January 2013 6am\n", argv[0], argv[1] );
		return 1 ;
	}
	return 0 ;
}

static inline int set_test_filetime( char * filename )
{
	static struct tm accessed_tm ; // zeroed at start of program

	struct utimbuf filetime_to_set ;
	time_t accessed_time ;

	accessed_tm.tm_hour = 6 ;
	accessed_tm.tm_mday = 1 ;	// 1st
	accessed_tm.tm_mon = 0 ;		// Jan
	accessed_tm.tm_year = 113 ;	// 1900 + 113 = 2013
	accessed_time = mktime( &accessed_tm );

	filetime_to_set.actime = accessed_time ;
	filetime_to_set.modtime = accessed_time ;

	return utime( filename, &filetime_to_set ) ;
}
