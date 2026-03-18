/*
	utility.h	Utility functions.
	Copyright (c) 1996-2026 by Christopher Heng. All rights reserved.
*/

#if !defined(UTILITY_H_INCLUDED)
#define	UTILITY_H_INCLUDED

#include "config.h"	// NORETURN

#if defined(__cplusplus)
extern "C" {
#endif

/* function declarations */
#if defined(__WATCOMC__)	/* errnomem() never returns */
#pragma aux errnomem aborts
#endif
extern NORETURN void errnomem ( int exitcode );
extern char * strip_path_prefix( char * orig_filename );
extern void * xmalloc ( size_t len );
extern char * xstrdup( const char * s );

#if defined(__cplusplus)
}
#endif


#endif
