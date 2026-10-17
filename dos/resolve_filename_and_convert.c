/*
	resolve_filename_and_convert.c
	Copyright 2026 Christopher Heng. All rights reserved.

	resolve_filename_and_convert() for MSDOS
*/

#include "config.h"
#include "tofrodos.h"

#if defined(MSDOS)
/*
	On MSDOS, this function does nothing, since DOS does
	not have symbolic links.
*/
int resolve_filename_and_convert ( char * filename )
{
	return process_file( filename );
}
#endif
