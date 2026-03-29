/*
	copy_file.h
	Copyright 2026 Christopher Heng. All rights reserved.
*/

#if !defined(COPY_FILE_H_INCLUDED)
#define	COPY_FILE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	cp_dest_write_error = -4,
	cp_source_read_error = -3,
	cp_dest_open_error = -2,
	cp_source_open_error = -1,
	cp_no_error = 0
} copy_file_status_t ;

extern copy_file_status_t copy_file ( char * source_file, char * dest_file ) ;

#ifdef __cplusplus
}
#endif

#endif /* COPY_FILE_H_INCLUDED */
