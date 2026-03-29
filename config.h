/*
	config.h	Handles system dependencies.
	Copyright 1996-2026 by Christopher Heng. All rights reserved.
*/

/*
	I have not specifically designed this program so that it is
	portable across systems. The comments below might help if you
	are using anything other than the compilers I used to develop
	the program. Note that the comments and macros in this file
	about system dependencies are not necessarily exhaustive.

	1. These macros are defined for the following systems:
	System					Macros defined
	------					--------------
	LINUX					LINUX, UNIX
	MSDOS					MSDOS
	WIN32					WIN32

	2. You will need a getopt_long() implementation. If you don't have
	one, define USE_SUPPLIED_GETOPT. This will use my
	getopt_long() implementation in the lib subdirectory. You will have
	to modify your makefile to compile and link lib/getopt.c in
	that case, as well. Note that if you use Visual Studio or
	Open Watcom, all these are already done.

	3. If your system has <unistd.h>, define HAVE_UNISTD_H. This is
	usually relevant only for Unix systems and MinGW-w64.
	If you don't have unistd.h, you may have to declare some
	standard Unix functions that are usually found there, such as
	chown(), chmod(), etc.

	4. Note that on MSDOS and Windows systems, you will need _splitpath()
	and _makepath(). If you use DJGPP, you probably can get away
	with defining _splitpath() to call fnsplit(), etc. Otherwise,
	you will need to roll your own version. That said, I think all the
	commercial MSDOS/Windows C compilers have these functions.

	5. You will need mkstemp(). This is a standard function
	on Unix systems, and it is also included in MinGW-w64.
	I have supplied my own mkstemp() for use with compilers that
	don't have the function (namely Visual Studio and OpenWatcom).

	6. tofrodos.c assumes utime.h exists in <sys/utime.h> for
	Microsoft's compiler and Watcom C/C++ (which tries to emulate
	Microsoft's compiler closely). It assumes that all other compilers
	keep utime.h in the standard include directories which are accessible
	simply by including <utime.h>.
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
// Note that these typedefs are already defined in the "stable testing" versions of OpenWatcom from
// https://openwatcom.org/ftp/source/ow_portable_v2_stable.zip
// so __WATCOMC__ is not in the list here, but I'm actually not sure if they were defined in version 1.9.
// If you're compiling with OpenWatcom 1.9, and get errors about uid_t and gid_t being unknown, add
// "|| (defined(__WATCOMC__) && (__WATCOMC__ <= 1290))" to the list above, although I don't know if the
// stable testing version actually incremented the __WATCOMC__ macro. I have no idea about the
// OpenWatcom V2 Fork project, and whether it's the same codebase or not.
// (In addition, I'm not sure why MinGW-w64 does not have these typedefs.)
typedef int uid_t ;
typedef int gid_t ;
#endif

#if defined(_MSC_VER)

// the OpenWatcom stable testing version seems to have these, so they are now only needed
// for Visual Studio. If you are using OpenWatcom 1.9, you may need to add
// "|| (defined(__WATCOMC__) && (__WATCOMC__ <= 1290))" to the #if defined(_MSC_VER) line above.
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
