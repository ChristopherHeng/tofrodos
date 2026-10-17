/*
	append_to_path_prefix.c
	Copyright 2022-2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"
#include <stdlib.h>	// malloc(), size_t
#include <string.h> // strlen(), strcpy(), strcat()
#include "safely_add_size_t_operands.h"
#include "tofrodos.h"

/*
	char * append_to_path_prefix( char * prefix, char * filename, char * extension )
		- prefix cannot be NULL. If it is, the function returns NULL immediately without
		doing anything else.
		- Do not prefix the filename with a "/" or "\\", or the extension with a ".".
		This function will add those things, and it does so without checking.
		- filename can be NULL, in which case, the function assumes that prefix already
		has the filename, and simply adds a "." followed by the extension.
		- extension can also be NULL.

	Note that filename[.extension] can be a directory name. The function
	doesn't really care whether "filename" is a file or a directory. It just
	allocates memory and appends stuff.

	Returns:
	- on success, the malloc()ed concatenated string is returned.
	You should free() it when done.

	- on failure, NULL is returned. This is returned if (1) there is insufficient
	memory, (2) if concatenating the various components result in a string that
	exceeds size_t, or (3) if prefix is NULL.
*/
char * append_to_path_prefix( char * prefix, char * filename, char * extension )
{
	size_t len ;
	size_t prefix_len ;
	size_t filename_len ;
	size_t ext_len ;
	char * s ;
	char * prefix_temp_ptr ;
	int is_relative_drive ; // only useful in Windows, will be always zero in Linux

	do { // dummy loop
		if (prefix == NULL) {
			s = NULL ;
			break ;
		}
	
		prefix_len = strlen( prefix ); // still need to +1 for final null byte
		filename_len = (filename != NULL) ? strlen( filename ) : 0 ; // still need to +1 for slash before filename
		ext_len = (extension != NULL) ? strlen( extension ) : 0 ; // still need to +1 for "." before extension

		/*
			We want to add the lengths of the prefix, filename and extensions together, and +3 for
			'/' and '.' and the final null byte. But since the different components are obtained from
			the user, we need to make sure that adding won't overflow a size_t.
		*/
		if (safely_add_size_t_operands( &len, prefix_len, filename_len ) ||
			safely_add_size_t_operands( &len, len, ext_len ) ||
			safely_add_size_t_operands( &len, len, 3 )) {

			s = NULL ;
			break ;

		}
	
		if ((s = malloc( len )) == NULL) {
			break ;
		}
		strcpy( s, prefix );
	
		is_relative_drive = 0 ;
	
		if (prefix_len && filename != NULL) {
			/*
				strip trailing slashes/backslashes from prefix if any. (It's never needed if no filename
				is to be added, and indeed must not be done, just in case.)
			*/
			prefix_temp_ptr = s + prefix_len - 1 ;
	
			if (*prefix_temp_ptr == ':') {
				// check for the case where a drive is specified for prefix and mark it now, for later use
				// WARNING: is_relative_drive is therefore only valid when filename != NULL
#if defined(WIN32) || defined(MSDOS)	// ifdef in case ':' is a valid char for a normal pathname on Linux
				is_relative_drive = 1 ;
#endif
				// note that this condition is empty on POSIX systems
			}
			else {
				/*
					Strip any trailing slashes or backslashes from prefix.
	
					If this is the root directory, we'll effectively be making this a relative directory at this
					point. However, we will be adding a slash later, so when everything's done, all will be back
					to its original state.
				*/
				prefix_len = strip_trailing_dir_separators( s, prefix_len );
			}
		}
	
		if (filename != NULL) {
			if (prefix_len && !is_relative_drive) {
				// only add a slash separating the prefix and filename if the prefix is not a drive without
				// a directory separator (ie, don't change a prefix of (say) "d:" (drive d: with its current working
				// directory of whatever-it-may-be) to "d:\", and if there is a prefix in the first place
#if defined(WIN32) || defined(MSDOS)
				strcat( s, "\\" );
#else
				strcat( s, "/" );
#endif
			}
			strcat( s, filename );
		}
		if (extension != NULL) {
			strcat( s, "." );
			strcat( s, extension );
		}
	} while (0);
	return s ;
}
