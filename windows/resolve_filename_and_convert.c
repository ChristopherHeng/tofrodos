/*
	windows/resolve_filename_and_convert.c
	Copyright 2026 Christopher Heng. All rights reserved.

	resolve_filename_and_convert() for Windows.
*/

#include "config.h"

#include <assert.h>		// static_assert()
#include <limits.h>		// ULONG_MAX
#include <stdlib.h>		// free(), NULL
#include <windows.h>	// FindFirstFile(), FindClose(), CreateFile(), GetFinalPathNameByHandle(),
						// INVALID_HANDLE_VALUE, FILE_ATTRIBUTE_REPARSE_POINT, IO_REPARSE_TAG_SYMLINK,
						// GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
						// FILE_NAME_NORMALIZED, DWORD

#include "emsg.h"
#include "tofrodos.h"
#include "utility.h"

#if defined(WIN32)

static inline int safely_add_dword_operands( DWORD * result, DWORD first, DWORD second );

int resolve_filename_and_convert ( char * filename )
{
	HANDLE hsearch ;
	HANDLE fh ;
	WIN32_FIND_DATA find_data ;
	DWORD len ;
	char dummy_buffer[1] ;	// buffer that is too small to contain any filename
							// (because I don't know if GetFinalPathByHandle() will accept NULL pointers)
	char * resolved_filename ;
	int error_resolving_symlink ;
	int retval ;

	error_resolving_symlink = 0 ;
	fh = INVALID_HANDLE_VALUE ;
	resolved_filename = NULL ;
	hsearch = FindFirstFile( filename, &find_data );
	do {
		if (hsearch != INVALID_HANDLE_VALUE) {
			if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
				(find_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK)) {
				// filename is a symlink

				// open it and let Windows resolve the symlink for us (by opening the real file pointed to by the link)
				fh = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ,
					NULL, // lpSecurityAttributes
					OPEN_EXISTING, // dwCreationDisposition
					FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
					NULL );
				if (fh == INVALID_HANDLE_VALUE) {
					// broken symlink?
					emsg ( EMSG_SYMLINK, filename );
					error_resolving_symlink = 1 ;
					break ;
				}
				// get the name of the file we opened; first find out how much memory we need to allocate
				len = GetFinalPathNameByHandleA( fh, dummy_buffer, 0, FILE_NAME_NORMALIZED );
				if (len == 0) {
					// function failed for some reason
					emsg ( EMSG_SYMLINK, filename );
					error_resolving_symlink = 1 ;
					break ;
				}
				// need to add 1 for the terminating null byte; do it the safe way
				if (safely_add_dword_operands( &len, len, 1 )) {
					// this should never happen since that means the path is more than 4 billion chars long
					emsg( EMSG_SYMLINK, filename );
					error_resolving_symlink = 1 ;
					break ;
				}
				// Just a little sanity check; this requires a compiler that supports C11's static_assert()
				// This test should pass on Windows because Windows compilers (Visual Studio and MinGW-w64's gcc)
				// define unsigned long (DWORD) as having the same size as unsigned int (size_t),
				// ie, 32-bits even on 64 bit systems.
				static_assert( (sizeof(size_t) >= sizeof(DWORD)), "size_t cannot hold all the values of DWORD" );
				resolved_filename = xmalloc( (size_t) len );
				// now really get the filename into our allocated space
				len = GetFinalPathNameByHandleA( fh, resolved_filename, len, FILE_NAME_NORMALIZED );
				if (len == 0) {
					// maybe something happened to the file between the time we called the function for the first time and now
					emsg ( EMSG_SYMLINK, filename );
					error_resolving_symlink = 1 ;
					break ;
				}
				else {
					if (verbose) {
						emsg( VERBOSE_SYMLINKSRC, filename, resolved_filename );
					}
				}
			} // if filename is a symlink
		}
	} while (0) ;

	if (fh != INVALID_HANDLE_VALUE) {
		CloseHandle( fh );
	}
	if (hsearch != INVALID_HANDLE_VALUE) {
		FindClose( hsearch );
	}

	if (error_resolving_symlink) {
		retval = -1 ;
	}
	else {
		retval = process_file( (resolved_filename == NULL) ? filename : resolved_filename );
		if (resolved_filename != NULL) {
			free( resolved_filename );
		}
	}
	return retval ;
}

static inline int safely_add_dword_operands( DWORD * result, DWORD first, DWORD second )
{
	// DWORD is actually unsigned long in Windows
	// info from: https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
	static_assert( (sizeof(unsigned long) == sizeof(DWORD)), "DWORD is not unsigned long as assumed" );
	if ((ULONG_MAX - first) >= second) {
		*result = first + second ;
		return 0 ;
	}
	else
		return -1 ;
}

#endif
