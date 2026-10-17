/*
	safely_add_size_t_operands.h
	Copyright 2026 by Christopher Heng. All rights reserved.
*/

#if !defined(SAFELY_ADD_SIZE_T_OPERANDS_INCLUDED)
#define	SAFELY_ADD_SIZE_T_OPERANDS_INCLUDED

#include <stdint.h>	// SIZE_MAX

/*
	Avoid wraparound when adding 2 size_t values. Returns 0 on success and the sum is
	stored in *result; if the operation will cause a wraparound, it will not be
	performed and -1 is returned.
*/
static inline int safely_add_size_t_operands( size_t * result, size_t first, size_t second )
{
	if ((SIZE_MAX - first) >= second) {
		*result = first + second ;
		return 0 ;
	}
	else
		return -1 ;
}

#endif
