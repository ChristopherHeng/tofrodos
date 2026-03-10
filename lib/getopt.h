/*
	getopt.h
	Copyright 1996-2026 by Christopher Heng. All rights reserved.
*/

#if !defined(GETOPT_H_INCLUDED)
#define GETOPT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma warning( push )
// 4 bytes padding added after data member has_arg
// 4 bytes padding added after data member val
#pragma warning(disable: 4820)
#endif
// used by getopt_long()
struct option {
	char * name ;
	enum { no_argument = 0, required_argument = 1, optional_argument = 2 } has_arg ;
	int * flag ;
	int val ;
};
#if defined(_MSC_VER)
#pragma warning( pop )
#endif

// global variables
extern char * optarg;	/* argument if option has one */
extern int opterr;		/* 0 = don't print err msg, 1 = print */
extern int optind;		/* next argument to process */
extern int optopt ;		/* current option */
extern int optreset ;	/* set by user to 1 if you want to reset getopt() */

// function declarations
extern int getopt_long ( int argc, char * const * argv, const char * optstring,
	const struct option * longopts, int * longindex );
extern int getopt( int argc, char * const * argv, const char * optstring );

#ifdef __cplusplus
}
#endif

#endif /* GETOPT_H_INCLUDED */
