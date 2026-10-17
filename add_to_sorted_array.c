/*
	add_to_sorted_array.c
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

#include "config.h"

#include <stdlib.h>	// realloc()
#include <string.h>	// strcmp()

#include "emsg.h"
#include "safely_add_size_t_operands.h"
#include "tofrodos.h"

#define	ARRAY_BLOCK_INCREMENT	100	// number of items (multiply this by sizeof(char*) to get size of block)

static inline void insert_in_sorted_order( sorted_array_t * array_header, char * item );

int add_to_sorted_array( sorted_array_t * array_header, char * item )
{
	int		retval ;
	size_t	len_to_allocate ;
	size_t	current_len ;
	char ** expanded_array_of_ptrs ;

	retval = 0 ;
	do {
		// This handles the situation where we run out of space to add items.
		// It also takes care of the initial call, where the array has not
		// been allocated yet.
		if (array_header->next_to_use == array_header->max) {
			current_len = array_header->max * sizeof( char * );
			if (safely_add_size_t_operands( &len_to_allocate, current_len,
				(ARRAY_BLOCK_INCREMENT * sizeof( char * )) )) {
				emsg( EMSG_TOO_MANY_EXT_OR_EXCLUDES );
				retval = -1 ;
				break ;
			}
			expanded_array_of_ptrs = realloc( array_header->array_of_ptrs, len_to_allocate );
			if (expanded_array_of_ptrs == NULL) {
				emsg( EMSG_NOMEM );
				retval = -1 ;
				break ;
			}
			array_header->array_of_ptrs = expanded_array_of_ptrs ;
			array_header->max += ARRAY_BLOCK_INCREMENT ;
				// It's always safe to just add since the len_to_allocate above would
				// wrap around long before this max value does; and the former has
				// already been checked above.
		}
		insert_in_sorted_order( array_header, item );
	} while (0);
	return retval ;
}

static inline void insert_in_sorted_order( sorted_array_t * array_header, char * item )
{
	size_t i ;
	char ** start_of_array ;

	start_of_array = array_header->array_of_ptrs ;
	i = array_header->next_to_use ; // save the index to the next empty slot
	array_header->next_to_use++ ;	// increment in anticipation of what we are about to do

	// Insert the item into the array in its correctly sorted position. This allows us to use
	// a binary search later, which may speed things up if there are lots
	// of filenames to process in our recursive descent through the directories.
	// Since the array is already sorted, we only need to maintain its sorted order by
	// making a hole in the right spot and plugging the new pointer in.
	// In effect, this is just the inner loop of insertion sort, which is probably more appropriate for
	// a small array like ours, where there are typically (much) fewer than 10 elements, than say, quicksort.
	// Note that the Windows and MSDOS versions do not need the array to be sorted for the
	// extensions array, (though it does for the file and dir exclusion arrays), but we do it anyway so
	// that we have fewer code paths to test.
	while ((i > 0) && (strcmp( start_of_array[i - 1], item ) > 0)) {
		start_of_array[i] = start_of_array[i - 1] ;
		i-- ;
	}
	start_of_array[i] = item ;
}
