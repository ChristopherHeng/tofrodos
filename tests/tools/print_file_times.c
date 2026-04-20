/*
	print_file_times.c
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static int use_local_time ;
static int no_dereference ;

static void convert_and_print( FILETIME * ftime, char * type_of_time );
static inline void print_file_times( char * filename );

int main ( int argc, char ** argv )
{
	PROCESS_LEAP_SECOND_INFO info = { 0 };
	int i ;
	int exit_code ;

	i = 1 ;
	while (i < argc) {	
		if (!strcmp( argv[i], "--localtime" )) {
			use_local_time = 1 ;
		}
		else if (!strcmp( argv[i], "--no-dereference" )) {
			no_dereference = 1 ;
		}
		else {
			// All options must precede the filename in this simple command line parser.
			// If you need something more, use getopt_long().
			break ;
		}
		i++ ;
	}

	if (argc == i) {
		fprintf( stderr, "Usage: print_file_times [--localtime] [--no-dereference] filename\n" );
		exit_code = EXIT_FAILURE ;
	}
	else {
		(void) SetProcessInformation( GetCurrentProcess(), ProcessLeapSecondInfo, &info, sizeof( info ) ) ;
		print_file_times( argv[i] );
		exit_code = EXIT_SUCCESS ;
	}

	return exit_code ;
}

static inline void print_file_times( char * filename )
{
	HANDLE fh ;
	FILETIME atime ;
	FILETIME mtime ;

	do {
		fh = CreateFile( filename,
				GENERIC_READ, // dwDesiredAccess
				FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, // dwShareMode
				NULL, // lpSecurityAttributes
				OPEN_EXISTING, // dwCreationDisposition
				(no_dereference ? FILE_FLAG_OPEN_REPARSE_POINT : FILE_ATTRIBUTE_NORMAL), // dwFlagsAndAttributes
				NULL // hTemplateFile
				) ;
		if (fh == INVALID_HANDLE_VALUE) {
			fprintf( stderr, "Unable to open \"%s\"\n", filename );
			break ;
		}
		if (GetFileTime( fh, NULL, &atime, &mtime ) == FALSE) {
			fprintf( stderr, "Unable to get file time from \"%s\"\n", filename );
			break ;
		}
		convert_and_print( &atime, "atime" );
		convert_and_print( &mtime, "mtime" );
	} while (0);

	if (fh != INVALID_HANDLE_VALUE) {
		CloseHandle( fh );
	}
	return ;

}

static void convert_and_print( FILETIME * ftime, char * type_of_time )
{
	SYSTEMTIME	time_info ;
	SYSTEMTIME	local_info ;

	if (FileTimeToSystemTime( ftime, &time_info ) == FALSE) {
		fprintf( stderr, "FileTimeToSystemTime() failed converting %s\n", type_of_time );
	}
	else {
		if (use_local_time) {
			if (SystemTimeToTzSpecificLocalTime( NULL, &time_info, &local_info ) == FALSE) {
				fprintf( stderr, "SystemTimeToTzSpecificLocalTime() failed while converting %s\n", type_of_time );
			}
			else {
				time_info = local_info ;
			}
		}
		fprintf( stdout, "%s: %d-%d-%d %d:%d:%d.%d\n",
			type_of_time, (int) time_info.wYear, (int) time_info.wMonth, (int) time_info.wDay,
			(int) time_info.wHour, (int) time_info.wMinute, (int) time_info.wSecond,
			(int) time_info.wMilliseconds );
	}
	return ;
}
