/*
	convert_file.c
	Copyright 1996-2026 Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <stdio.h>	// FILE *, putc(), getc(), feof(), EOF

#include "emsg.h"
#include "tofrodos.h"

/*
	convert_file()

	Does the actual work of converting infp to outfp.

	If direction is DIRECTION_DOSTOUNIX, "\r\n" pairs will be converted to
	'\n'. However, standalone '\r' without a '\n' immediately
	following will not be eliminated.

	If direction is DIRECTION_UNIXTODOS, '\n' will be converted to "\r\n".
	However "\r\n" pairs are not converted to '\r\r\n'.

	Returns 0 on success, -1 on error.
*/
int convert_file ( FILE * infp, FILE * outfp )
{
	int prevch ;
	int c ;

	/* actually it is very simple to do the conversion in DOS/WIN32 */
	/* because the stdio library does this work automatically for */
	/* us. But since we want this program to work on Linux as */
	/* well, a little bit of work stands before us (but only a little). */

	prevch = EOF ;

	if (direction == DIRECTION_UNIX2DOS) {
		/* basically we convert all newlines to "\r\n" unless */
		/* the file is already in "\r\n" format. The problem here */
		/* is when you have special situations like a Unix */
		/* text file with lines that have a '\r' just */
		/* before a '\n'. These lines will */
		/* not be converted to "\r\r\n" since the function */
		/* below assumes the line has already been converted. */
		while ( (c = getc( infp )) != EOF ) {
			if (c == '\n' && prevch != '\r') {
				if (putc( '\r', outfp ) == EOF)
					break ;
			}
			/* always emit the current character */
			if (putc( c, outfp ) == EOF)
				break ;
			prevch = c ;                        	 
		}
	}
	else if (direction == DIRECTION_DOS2UNIX) {
		/* basically we withhold emitting any '\r' until we */
		/* are sure that the next character is not a '\n'. */
		/* If it is not, we emit the '\r'. If it is, we */
		/* only emit the '\n'. */
		while ( (c = getc( infp )) != EOF ) {
			if (prevch == '\r') {
				/* '\r' is a special case because we don't */
				/* emit a '\r' until the next character */
				/* has been read */
				if (c == '\n') { /* a "\r\n" pair */
					/* discard previous '\r' and */
					/* just put the '\n' */
					if (putc( c, outfp ) == EOF)
						break ;
				}
				else {	/* prevch was a standalone '\r' but the current char is not '\n' */
						/* emit the standalone '\r' */
					if (putc( '\r', outfp ) == EOF)
						break ;
					/* emit the current character if */
					/* it is not a '\r' */
					if (c != '\r') {
						if (putc( c, outfp ) == EOF)
							break ;
					}
				}
			}
			else { /* prevch was not '\r' */
				/* emit current character if it is not */
				/* a '\r' */
				if (c != '\r') {
					if (putc( c, outfp ) == EOF)
						break ;
				}
			}
			prevch = c ;
		}
	}
	else {
		emsg( EMSG_INTERNAL, EINTNL_DIRECTION );
		return -1 ;
	}

	/* if we reach here, either we've reached an EOF or an error */
	/* occurred. */
	if (!feof( infp )) { /* error */
		emsg( EMSG_CONVERT, current_input_filename );
		return -1 ;
	}
	return 0 ;
}
