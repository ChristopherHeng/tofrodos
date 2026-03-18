/*
	resolve_filename_and_convert.c
	Copyright 2026 Christopher Heng. All rights reserved.

	resolve_filename_and_convert() for MSDOS
*/

// The include paths below specify the parent directory ("..")
// because I haven't figured out if WMAKE has a macro like the $(MAKEDIR) of NMAKE,
// or if there is a way to generate such a value the way you can with GNU make.
// Basically, I want to add the path of the parent directory to the -i option
// (include directory) of the compiler. In the end, the following lines were
// just the lazy way out.
#include "../config.h"
#include "../tofrodos.h"

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
