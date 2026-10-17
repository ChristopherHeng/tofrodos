/*
	item_is_in_array.c
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"
#include <stdlib.h>		// bsearch()
#include <string.h>		// strcmp() or strcasecmp()
#include "tofrodos.h"

#if defined(WIN32) || (MSDOS)
	// Windows has a case-insensitive file system (or at least NTFS/FAT does)
#define	COMPARE_PATH(a,b) strcasecmp(a,b)
#else
	// POSIX systems have case-sensitive file systems
#define	COMPARE_PATH(a,b) strcmp(a,b)
#endif

static int compare_strings( const void * first, const void * second );

int item_is_in_array( sorted_array_t * array_header, char * item )
{
	return ((bsearch( (const void *) &item, array_header->array_of_ptrs, array_header->next_to_use,
		sizeof( char * ), compare_strings ) == NULL) ? 0 : 1 );
}

static int compare_strings( const void * first, const void * second )
{
	return COMPARE_PATH( *(const char **)first, *(const char **)second );
}
