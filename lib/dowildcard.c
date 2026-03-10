/*
	dowildcard.c

	This file is just to enable wildcard processing for MinGW-w64.
	Wildcard processing is disabled by default in newer versions of MInGW-w64.
*/


#if defined(__MINGW32__)
/*
	-1	enable wildcard expansion for main()'s argv[]
	0	disable wildcard expansion for main()'s argv[]
*/
int _dowildcard = -1 ;
#endif
