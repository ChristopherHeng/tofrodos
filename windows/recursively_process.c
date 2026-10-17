/*
	windows/recursively_process.c
	Copyright 2022-2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"
#include <stdlib.h>	// free(),
#include <string.h>	// strlen(), strcmp()
#include <windows.h>	// FindFirstFile(), FindNextFile(), HANDLE, WIN32_FIND_DATA, MAX_PATH

#if defined(USE_SUPPLIED_GETOPT)
#include "getopt.h"	// optind
#else
#include <getopt.h>	// optind
#endif

#include "emsg.h"
#include "tofrodos.h"

static int recurse_into_directory( char * dirname );

int recursively_process( int argc, char ** argv )
{
	static char current_dir[] = "" ; // yes; empty string; we don't need it to be "."
	int ret_val ;
	int i ;

	if (argc == optind) {
		ret_val = recurse_into_directory( current_dir );
	}
	else {
		ret_val = 0 ;
		i = optind ;
		while (i < argc) {
			if (((ret_val = recurse_into_directory( argv[optind] )) != 0) && abortonerr) {
				break ;
			}
			i++ ;
		}
	}
	return ret_val ;
}


static int recurse_into_directory( char * dirname )
{
	size_t cur_ext ;
	char * pattern ;
	char * filepath ;
	char * child_dir ;
	int	breakout_and_return ;
	int ret_val ;
	HANDLE hsearch ;
	WIN32_FIND_DATA find_file_data ;

	if (verbose) {
		if (*dirname == '\0') {
			emsg( VERBOSE_RECURSE_CURDIR );
		}
		else {
			emsg( VERBOSE_RECURSE_DIR, dirname );
		}
	}

	pattern = NULL ;
	filepath = NULL ;
	breakout_and_return = 0 ;
	ret_val = 0 ;

	// process all relevant files in the current dirname
	for (cur_ext = 0; (!breakout_and_return && (cur_ext < extensions.next_to_use)); cur_ext++) {
		if ((pattern = append_to_path_prefix( dirname, "*", extensions.array_of_ptrs[cur_ext] )) == NULL) {
			emsg( EMSG_NOMEM );
			ret_val = -1 ;
			breakout_and_return = 1 ;
			break ;
		}
		if (strlen( pattern ) >= MAX_PATH) {
			/*
				MAX_PATH is 260 chars
				https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd
				and FindFirstFileA(), which is what we're effectively calling here, cannot handle
				more than that. FindFirstFileW() can, but then we will have to convert to UTF-16.
			*/
			emsg( EMSG_DIR_TOO_DEEP, dirname );
			ret_val = -1 ;
			breakout_and_return = 1 ;
			break ; // breaks out of for loop, pattern remains allocated
		}
		hsearch = FindFirstFile( pattern, &find_file_data );
		if (hsearch != INVALID_HANDLE_VALUE) {
			do {
				if (!(find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					if (!item_is_in_array( &file_excludes, find_file_data.cFileName )) {
						if ((filepath = append_to_path_prefix( dirname, find_file_data.cFileName, NULL )) == NULL) {
							emsg( EMSG_NOMEM );
							ret_val = -1 ;
							breakout_and_return = 1 ;
							break ;
						}
					    if (verbose) {
							emsg( VERBOSE_CONVERTING, filepath );
						}
						if (resolve_filename_and_convert( filepath ) && abortonerr) {
							breakout_and_return = 1 ;
							ret_val = -1 ;
							// don't do the break here so that we can free filepath first
						}
						free( filepath );
						filepath = NULL ;
						if (breakout_and_return) {
							break ;
						}
					}
					else {
						if (verbose) {
							emsg( VERBOSE_RECURSE_SKIP_FILE, find_file_data.cFileName );
						}
					}
				}
			} while (FindNextFile( hsearch, &find_file_data ) != 0);
		}
		free( pattern );
		pattern = NULL ;
		FindClose( hsearch );
	} // for loop to process all files in current directory

	// clean up in case we broke out due to errors
	if (pattern != NULL) {
		free( pattern );
		pattern = NULL ;
	}

	// now descend into subdirectories if any
	if (!breakout_and_return) {
		// no need to check if < MAX_PATH, since this string is shorter than the earlier check
		if ((pattern = append_to_path_prefix( dirname, "*", NULL )) == NULL) {
			emsg( EMSG_NOMEM );
			ret_val = -1 ;
		}
		else {
			hsearch = FindFirstFile( pattern, &find_file_data );
			if (hsearch != INVALID_HANDLE_VALUE) {
				child_dir = NULL ;
				do {
					if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						// Note: at this time, we don't check for FILE_ATTRIBUTE_REPARSE_POINT which could indicate a symlink.
						// In other words, the code here should always follow symlinks.

						// skip "." and ".."
						if (!strcmp( ".", find_file_data.cFileName ) || !strcmp( "..", find_file_data.cFileName )) {
							continue ;
						}
						// descend into directory
						if (!item_is_in_array( &dir_excludes, find_file_data.cFileName )) {
							if ((child_dir = append_to_path_prefix( dirname, find_file_data.cFileName, NULL )) == NULL) {
								emsg( EMSG_NOMEM );
								ret_val = -1 ;
								break ;
							}
							ret_val = recurse_into_directory( child_dir ) ;
							free( child_dir ); // prepare for next iteration through the loop
							child_dir = NULL ;
							if (ret_val) {
								// percolate the ret_val up the chain of recursive calls
								break ;
							}
						}
						else {
							if (verbose) {
								emsg( VERBOSE_RECURSE_SKIP_DIR, find_file_data.cFileName );
							}
						}
					}
				} while (FindNextFile( hsearch, &find_file_data ) != 0);
			}
			free( pattern );
			FindClose( hsearch );
		}
	}

	return ret_val ;
}
