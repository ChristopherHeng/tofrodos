/*
	strip_trailing_dir_separators.c
	Copyright 2022-2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"
#include "tofrodos.h"	// size_t also defined because this hdr includes stdio.h

size_t strip_trailing_dir_separators( char * path, size_t len )
{
	char * s ;

	if (len) {
		s = path + len - 1;
		while (1) {
#if defined(WIN32)
			if ((*s == '/') || (*s == '\\')) {
#elif defined(MSDOS)
			if (*s == '\\') {
#else
			if (*s == '/') {
#endif
				*s = '\0' ;
				len-- ;
			}
			else {
				// if there is no separator, we're done
				break ;
			}
			// this is reached only if a separator was encountered above;
			if (len > 0) {
				s-- ;
			}
			else {
				// we reached the beginning of the string
				break ;
			}
		}
	}
	return len ;
}
