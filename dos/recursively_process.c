/*
	dos/recursively_process.c
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <dos.h>	// _dos_findfirst(), _dos_findnext()
#include <stdlib.h>	// free()
#include <string.h>	// strcmp()

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
	int	breakout_and_return ;
	char * pattern ;
	char * file_path ;
	char * child_dir ;
	struct find_t file_find_data ;
	int ff_retval ;
	int ret_val ;

	if (verbose) {
		if (*dirname == '\0') {
			emsg( VERBOSE_RECURSE_CURDIR );
		}
		else {
			emsg( VERBOSE_RECURSE_DIR, dirname );
		}
	}

	breakout_and_return = 0 ;
	ret_val = 0 ;
	pattern = NULL ;
	file_path = NULL ;

	// process all relevant files in the current dirname
	for (cur_ext = 0; (!breakout_and_return && (cur_ext < extensions.next_to_use)); cur_ext++) {
		if ((pattern = append_to_path_prefix( dirname, "*", extensions.array_of_ptrs[cur_ext] )) == NULL) {
			emsg( EMSG_NOMEM );
			ret_val = -1 ;
			breakout_and_return = 1 ;
			break ;
		}
		ff_retval = _dos_findfirst( pattern, _A_NORMAL|_A_ARCH|_A_RDONLY, &file_find_data );
		free( pattern );
		pattern = NULL ;
		if (ff_retval) {
			// no such file or some other error
			continue ; // skip to the next extension
		}
		do {
			if (item_is_in_array( &file_excludes, file_find_data.name )) {
				// file is excluded
				if (verbose) {
					emsg( VERBOSE_RECURSE_SKIP_FILE, file_find_data.name );
				}
				// don't do anything else (since it's excluded)
			}
			else {
					if ((file_path = append_to_path_prefix( dirname, file_find_data.name, NULL )) == NULL) {
						emsg( EMSG_NOMEM );
						breakout_and_return = 1 ;
						ret_val = -1 ;
						// we'll break at the end of the do/while()
					}
					else {
					    if (verbose) {
							emsg( VERBOSE_CONVERTING, file_path );
						}
						if (process_file( file_path ) && abortonerr) {
							breakout_and_return = 1 ;
							ret_val = -1 ;
							// don't break here, so that we can free file_path
							// break will be done at the end of the do/while()
						}
						free( file_path );
						file_path = NULL ;
					}
					// nothing should be here (unless you check breakout_and_return beforehand)
			}
		} while (!breakout_and_return && !_dos_findnext( &file_find_data )) ;
	} // for loop

	// Since this is DOS-specific, we don't need to call _dos_findclose(). Per Watcom's docs, it's
	// only needed for Windows and OS/2.

	// now process subdirectories
	do {
		if (!breakout_and_return) {

			if ((pattern = append_to_path_prefix( dirname, "*.*", NULL )) == NULL) {
				emsg( EMSG_NOMEM );
				ret_val = -1 ;
				break ;
			}
			ff_retval = _dos_findfirst( pattern, _A_SUBDIR, &file_find_data );
			free( pattern );
			pattern = NULL ;
			if (ff_retval) {
				// no subdirectory to process; we're done and can return
				// (note that this is not an error, so we don't set ret_val to -1)
				break ;
			}
			child_dir = NULL ;
			do {
				// despite our use of _A_SUBDIR, normal attribute files (_A_NORMAL) will still be
				// returned from the search; see the old Microsoft Knowledgebase article Q43144
				// A copy can be found here: https://jeffpar.github.io/kbarchive/kb/043/Q43144/
				// So we have to specifically check if the current dir entry is actually a subdir.
				if (file_find_data.attrib & _A_SUBDIR) { // this is a subdirectory
					// skip "." and ".."
					if (!strcmp( ".", file_find_data.name ) || !strcmp( "..", file_find_data.name )) {
						continue ;
					}
					// check if it's in the exclusion list
					if (!item_is_in_array( &dir_excludes, file_find_data.name )) {
						if ((child_dir = append_to_path_prefix( dirname, file_find_data.name, NULL )) == NULL) {
							emsg( EMSG_NOMEM );
							ret_val = -1 ;
							break ;
						}
						ret_val = recurse_into_directory( child_dir ) ;
						free( child_dir ); // prepare for next iteration through the loop
						child_dir = NULL ;
						if (ret_val) { // there was an error and abortonerr is 1
							// percolate the ret_val up the stack of recursive calls
							break ;
						}
						// otherwise, no error, carry on with the other entries in this subdir
					}
					else { // dir is excluded
						if (verbose) {
							emsg( VERBOSE_RECURSE_SKIP_DIR, file_find_data.name );
						}
						// do nothing else, since it's excluded; carry on with the other entries in this subdir
					}
				}
			} while (!_dos_findnext( &file_find_data )) ;

			// Since this is DOS-specific, we don't need to call _dos_findclose(). Per Watcom's docs, it's
			// only needed for Windows and OS/2.

		} // if (!breakout_and_return)

	} while (0);

	return ret_val ;
}
