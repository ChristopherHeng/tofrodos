/*
	posix/recursively_process.c
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

/*
	There are a number of functions available to recurse a dir tree, all with
	disadvantages when trying to write a program that can compile and run on
	a variety of Unix-like systems.

	1. nftw() - Unfortunately, the BSDs and macOS only support the POSIX-mandated
	features of this function. POSIX does not provide a built-in way to skip
	a subdirectory which we need to do when --exclude-dir is used. The GNU version
	does, via a GNU-specific FTW_SKIP_SUBTREE return value, but alas, this
	extension is missing on the BSDs and macOS.

	2. fts functions - This is a set of functions from BSD that are also
	supported by glibc 2+, which means they work on Linux, the BSDs, and macOS.
	Unfortunately, they aren't in the POSIX specs, so they are not guaranteed
	to be on every Unix-like system.

	3. opendir()/readdir() and family - These are POSIX functions, which
	probably means they are ubiquitous. Unfortunately, they are somewhat
	bare bones, and I have to implement lots of stuff myself (with the attendant
	burden of having to test all possible scenarios), so this is a route
	I prefer not to take.
*/

#include "config.h"

#include <string.h>	// strerror(), strrchr()
#include <sys/types.h>
#include <sys/stat.h>	// stat()
#include <fts.h>

#if defined(USE_SUPPLIED_GETOPT)
#include "getopt.h"	// optind
#else
#include <getopt.h>	// optind
#endif

#include "emsg.h"
#include "tofrodos.h"

static inline int has_included_extension( char * filename );

int recursively_process( int argc, char ** argv )
{
	static char * current_dir[] = { ".", NULL };
	FTS * fts_handle ;
	FTSENT * node ;
	char ** arg_list ;
	int ret_val ;
	struct stat statbuf ;

	if (argc == optind) {
		arg_list = current_dir ;
	}
	else {
		arg_list = &argv[optind] ;
	}

	ret_val = 0 ;
	fts_handle = fts_open( arg_list, FTS_NOCHDIR | FTS_PHYSICAL, NULL );

	if (fts_handle != NULL) {
		while (!ret_val && ((node = fts_read( fts_handle )) != NULL )) {
			switch( node->fts_info ) {
				case FTS_D:
					if (item_is_in_array( &dir_excludes, node->fts_name )) {
						// this dir has been excluded by the user
						if (fts_set( fts_handle, node, FTS_SKIP )) { // tell the fts functions to skip it
							// huh? fts_set() does not recognize FTS_SKIP as an instruction/option?
							emsg( EMSG_FTS_ERROR, node->fts_name, strerror( node->fts_errno ) );
							ret_val = -1 ;
						}
						else if (verbose) {
							emsg( VERBOSE_RECURSE_SKIP_DIR, node->fts_name );
						}
					}
					else if (verbose) {
						// otherwise we will proceed with it (default, so don't need to do anything unless
						// verbose is on)
						emsg( VERBOSE_RECURSE_DIR, node->fts_name );
					}
					break ;
				case FTS_DC: // dir that caused a cycle in the tree; ignore; it will be skipped automatically
					break ;
				case FTS_DNR:
					// directory cannot be read
					// By default, we just silently skip it (even if abortonerr is set, since this
					// is not an error in processing a file); ie, we treat it as auto-resolved.
					// if --verbose, print a message just as we would when skipping directories.
					if (verbose) {
						emsg( VERBOSE_RECURSE_SKIP_UNREADABLE_DIR, node->fts_name );
					}
					break ;
				case FTS_DOT: // "." or ".."
					break ;
				case FTS_DP: // second call for a directory that we have already processed
					break ;
				case FTS_ERR:
					// fts_read() returned an error; node->fts_errno has the errno code
					emsg( EMSG_FTS_ERROR, node->fts_name, strerror( node->fts_errno ) );
					if (abortonerr) {
						ret_val = -1 ;
					}
					break ;
				case FTS_F:
					if (!item_is_in_array( &file_excludes, node->fts_name )) {
						// file is not specifically excluded

						if (has_included_extension( node->fts_name )) {
							// file has one of the valid extensions

							// node->fts_path is guaranteed to be null-terminated only for the file most recently
							// returned by fts_read(); since fts_read() and fts_set() are the only functions we call
							// (for example, we don't call fts_children()), this should be fine.
							if (verbose) {
								emsg( VERBOSE_CONVERTING, node->fts_name );
							}
							// we are calling process_file() directly instead of resolve_filename_and_convert()
							// because we know for sure that this is not a symbolic link, since fts_read() already
							// did the checking for us.
							if (process_file( node->fts_path ) && abortonerr) {
								ret_val = -1 ;	// to break out of the loop if user wants us to stop immediately on error
												// otherwise, we just go to the next file
							}
						}
					}
					else {
						// excluded file
						if (verbose) {
							emsg( VERBOSE_RECURSE_SKIP_FILE, node->fts_name );
						}
					}
					break ;
				case FTS_NS:
					// stat() failed for this file
					emsg( EMSG_ACCESSFILE, node->fts_name );
					if (abortonerr) {
						ret_val = -1 ;
					}
					break ;
				case FTS_NSOK:	// this won't occur, since we asked for stat() to be done
					break ;
				case FTS_SL:
					// need to check if this is a directory or file, and check against our inclusion/exclusion lists
					// use stat() to get info about the target of the symlink
					if (stat( node->fts_path, &statbuf )) {
						emsg( EMSG_ACCESSFILE, node->fts_path );
						ret_val = -1 ;
					}
					else {
						if (S_ISDIR( statbuf.st_mode )) {
							// symbolic link to a directory
							if (!item_is_in_array( &dir_excludes, node->fts_name )) {
								// if it's not excluded, flag for fts_read() to follow the link in the next call
								if (fts_set( fts_handle, node, FTS_FOLLOW )) {
									emsg( EMSG_FTS_ERROR, node->fts_name, strerror( node->fts_errno ) );
									ret_val = -1 ;
								}
							}
						}
						else if (S_ISREG( statbuf.st_mode )) {
							// it's a link to a regular file
							if (!item_is_in_array( &file_excludes, node->fts_name )) {
								// symbolic link's name is not excluded
								if (has_included_extension( node->fts_name )) {
									// symbolic link's name has the right extension
									if (verbose) {
										emsg( VERBOSE_CONVERTING, node->fts_name );
									}
									if (resolve_filename_and_convert( node->fts_path ) && abortonerr) {
										ret_val = -1 ;
									}
								}
							}
							else {
								// excluded file
								if (verbose) {
									emsg( VERBOSE_RECURSE_SKIP_FILE, node->fts_name );
								}
							}
						}
						else {
							// not a dir or a file (could be block or char special file or FIFO);
							emsg( EMSG_SYMLINK_UNKNOWN, node->fts_path );
							if (abortonerr) {
								ret_val = -1 ;
							}
						}
					}
					break ;
				case FTS_SLNONE: // broken symlink
					emsg( EMSG_BROKEN_SYMLINK, node->fts_path );
					if (abortonerr) {
						ret_val = -1 ;
					}
					break ;
				case FTS_DEFAULT: // anything other than the above values
				default:
					// ignore
					break ;
			}
		}
		fts_close( fts_handle );
	}
	else {
		emsg( EMSG_FTS_OPEN );
		ret_val = -1 ;
	}

	return ret_val ;
}

static inline int has_included_extension( char * filename )
{
	char * s ;
	int ret_val ;

	ret_val = 0 ;
	s = strrchr( filename, '.' );
	if (s != NULL) {
		s++ ;
		if (item_is_in_array( &extensions, s )) {
			ret_val = 1 ;
		}
	}
	return ret_val ;
}
