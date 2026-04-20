/*
	copy_file.c
	Copyright 2026 Christopher Heng. All rights reserved.
*/

#if defined(_MSC_VER)
// disable warnings about using certain CRT functions (like fopen(), etc)
#define _CRT_SECURE_NO_WARNINGS
// ompiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable: 5045)
#endif

#include <stdio.h>	// fopen(), fclose(), fread(), fwrite(), size_t, remove()
#include <stdlib.h>	// malloc(), free()

#if !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN32) && !defined(_WIN64) && !defined(__MSDOS__)
#include <sys/stat.h>	// umask(), mode_t
#endif

#include "copy_file.h"

#define	COPY_BUFFER_LEN	(32*1024)	// 32k buffer by default
#define	COPY_BUFFER_LEN_FALLBACK	(1*1024)	// 1k; used if the above cannot be allocated

/*
	copy_file

	Copies source_file to dest_file. If dest_file exists, its existing content is lost.

	Note: if an error occurs midway during the copy process, after the
	destination file was created (or truncated if it existed before),
	it (ie, the destination file) will be deleted before the function returns.

	This function only uses standard C functions (the stdio FILE functions, rather
	than the lower-level open/read/write/close functions), so that it is hopefully
	portable to other compilers/systems without my having to tweak it.
*/
copy_file_status_t copy_file ( char * source_file, char * dest_file )
{
	FILE * src_fp ;
	FILE * dest_fp ;
	size_t items_read ;
	size_t items_written ;
	char fallback_buffer[ COPY_BUFFER_LEN_FALLBACK ] ;
	char * buf ;
	size_t	buflen ;
	int must_free ;
	int dest_was_created ;
	copy_file_status_t retval ;
#if !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN32) && !defined(_WIN64) && !defined(__MSDOS__)
	mode_t old_umask ;
#endif

#if !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN32) && !defined(_WIN64) && !defined(__MSDOS__)
	old_umask = umask( S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ); // only allow user to read/write file
#endif

	do {
		src_fp = NULL ;
		dest_fp = NULL ;
		buf = NULL ;
		buflen = 0 ;
		must_free = 1 ;
		dest_was_created = 0 ;
		retval = cp_no_error ;
	
		buflen = COPY_BUFFER_LEN ;
		if ((buf = malloc( buflen )) == NULL) {
			// we never fail because we can't allocate the buffer; at worst, if
			// memory is tight, we just use our small stack-based fallback buffer
			buflen = COPY_BUFFER_LEN_FALLBACK ;
			buf = fallback_buffer ;
			must_free = 0 ; // so that we don't try to free() it later
		}

		// the "b" in the mode argument is needed for Windows/MSDOS, and is ignored
		// on POSIX systems
		if ((src_fp = fopen( source_file, "rb" )) == NULL) {
			retval = cp_source_open_error ;
			break ;
		}
		if ((dest_fp = fopen( dest_file, "wb" )) == NULL) {
			retval = cp_dest_open_error ;
			break ;
		}
		dest_was_created = 1 ; // *must* be after fopen() succeeds

		while (1) {
			items_read = fread( buf, 1, buflen, src_fp );
			if (items_read > 0) {
				items_written = fwrite( buf, 1, items_read, dest_fp );
				if (items_written < items_read) {
					retval = cp_dest_write_error ;
					break ; // break out of inner loop only
				}
			}
			else { // zero bytes read
				if (feof( src_fp )) {
					break ; // break out of inner loop only
				}
				else {
					retval = cp_source_read_error ;
					break ; // break out of inner loop only
				}
			}
		}
		// no code should be here (until the end of loop), since we may reach
		// here as a result of an error within the read/write loop, as well as
		// when we finish copying successfully. If you want to add something
		// here, make sure you check retval first.
	} while (0) ;

	if (src_fp != NULL)
		fclose( src_fp );
	if (dest_fp != NULL)
		fclose( dest_fp );
	if (must_free && (buf != NULL))
		free( buf );

	if ((retval != cp_no_error) && dest_was_created) {
		remove( dest_file );
	}

#if !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN32) && !defined(_WIN64) && !defined(__MSDOS__)
	(void) umask( old_umask );
#endif

	return retval ;
}
