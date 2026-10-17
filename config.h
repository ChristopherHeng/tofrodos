/*
	config.h	Handles system dependencies.
	Copyright 1996-2026 by Christopher Heng. All rights reserved.
*/

/*
	In general, Tofrodos was written to compile and run on Linux, Windows
	and MSDOS. It is also known to work on macOS although I don't have
	a Mac to test it myself. If you are porting to some other system,
	and find that it does not compile out of the box, you may want to
	start by tweaking this file.

	In addition, note the following:
	
	1. You will need a getopt_long() implementation. If you don't have
	one, define USE_SUPPLIED_GETOPT. This will use my
	getopt_long() implementation in the lib subdirectory. You will have
	to modify your makefile to compile and link lib/getopt.c in
	that case, as well. Note that if you use Visual Studio or
	Open Watcom, all these are already done.

	2. If your system has <unistd.h>, define HAVE_UNISTD_H. This is
	usually relevant only for Unix systems and MinGW-w64.
	If you don't have unistd.h, you may have to declare some
	standard Unix functions that are usually found there, such as
	chown(), chmod(), etc.

	3. You will need mkstemp(). This is a standard function
	on Unix-like systems, and it is also included in MinGW-w64.
	I have supplied my own mkstemp() for use with compilers that
	don't have the function (namely Visual Studio and OpenWatcom).
*/

#if !defined(CONFIG_H_INCLUDED)
#define	CONFIG_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

/* define the systems */
#if defined(__linux__)	/* (predefined) */
#if !defined(LINUX)
#define	LINUX
#endif
#if !defined(UNIX)
#define	UNIX		/* make sure this is defined */
#endif
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || \
	defined(__OpenBSD__) || defined(__NetBSD__) || defined(__GNU__)
	/* these systems seem to work like Linux. Note to self: __GNU__ == Hurd */
#if !defined(LINUX)
#define	LINUX
#endif
#if !defined(UNIX)
#define	UNIX		/* make sure this is defined */
#endif
#endif

#if defined(__MSDOS__)
#if !defined(MSDOS)
#define	MSDOS		/* make sure this is defined */
#endif
#endif

#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32) || defined(_WIN64)
#if !defined(WIN32)
#define	WIN32
#endif
#endif

#if defined(__APPLE__)
#if !defined(UNIX)
#define	UNIX
#endif
#endif

/* define what headers we have (based on the systems) */
#if defined(UNIX) || (__MINGW32__)
#define	HAVE_UNISTD_H
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define	USE_SUPPLIED_GETOPT
#endif

// for a function that never returns
// Note that for Watcom C, you have to use:
//		#pragma aux <function-name> aborts
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define NORETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#define	NORETURN [[noreturn]]
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define	NORETURN _Noreturn
#else
#define NORETURN
#endif

// define compatibility macros and typedefs so that we don't need so many ifdefs in the
// main code body (since they add clutter and make things hard to follow)

#if defined(_MSC_VER) || defined(__MINGW32__)
typedef int uid_t ;
typedef int gid_t ;
#endif

#if defined(_MSC_VER)
typedef int mode_t ;
#define	strcasecmp	stricmp

// Visual Studio has S_IREAD and S_IWRITE in sys/stat.h but not S_IRUSR and S_IWUSR
#define	S_IRUSR	S_IREAD
#define	S_IWUSR	S_IWRITE

// used for access()
#define	F_OK	0
#define	W_OK	2
#define	R_OK	4

// Visual Studio specific macros
// disable warnings about using certain CRT functions (like fopen(), strcpy(), etc)
#define _CRT_SECURE_NO_WARNINGS
// disable warnings about using standard POSIX function names
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#if defined(__cplusplus)
}
#endif

#endif	/* CONFIG_H_INCLUDED */
