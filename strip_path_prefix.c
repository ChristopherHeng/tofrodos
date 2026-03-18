/*
	strip_path_prefix.c
	Copyright 2021-2026 by Christopher Heng. All rights reserved.
*/

#include <string.h> // strlen()

#include "utility.h" // function declaration for this function

// note that we assume that orig_filename != NULL
char * strip_path_prefix( char * orig_filename )
{
	size_t orig_len ;
	char * s ;

	orig_len = strlen( orig_filename );
	if (orig_len < 1) {
		s = orig_filename ;
	}
	else {
		s = orig_filename + orig_len - 1 ; // point to last char of string
		while (s != orig_filename) {
#if defined(_WIN32) || defined(_WIN64)
			if ((*s == '/') || (*s == '\\') || (*s == ':')) {
#else	// POSIX
			if (*s == '/') {
#endif
				s++ ; // skip past the prefix to point to the filename proper
				break ;
			}
			s-- ;
		}
	}
	return s ;
}
