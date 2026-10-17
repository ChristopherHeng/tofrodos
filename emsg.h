/*
	emsg.h		Error messages.
	Copyright 1996-2026 by Christopher Heng. All rights reserved.
*/

#if !defined(EMSG_H_INCLUDED)
#define	EMSG_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

/* macros */
#define	EMSG_BAKFILENAME	"File cannot have a .bak extension when used with -b flag.\n"
#define	EMSG_CONVERT		"File read/write error while converting \"%s\".\n"
#define	EMSG_INTERNAL		"Internal error: %s.\n"
#define	EMSG_NOFILENAME		"Need to specify filename or redirect stdin.\n"
#define	EMSG_NOMEM			"Insufficient memory to run program.\n"
#define	EMSG_NOTEMPNAME		"Unable to generate temporary filename for converting \"%s\".\n"
#define	EMSG_OPENFILE		"Unable to open file \"%s\".\n"
#define	EMSG_SIGNAL			"Terminated by user.\n"
#define	EMSG_WRONGDOSVER	"Requires DOS 3.1 and above.\n"
#define	EMSG_ACCESSFILE		"Unable to access file \"%s\".\n"
#define	EMSG_NOTREADABLE	"No read permission for \"%s\".\n"
#if defined(MSDOS) || defined(WIN32)
#define	EMSG_NOTWRITEABLE	"\"%s\" is read-only. Use --force to force conversion.\n"
#else
#define	EMSG_NOTWRITEABLE	"No write permission for \"%s\". Use --force to force conversion.\n"
#endif
#define	EMSG_NOTWRITEABLE_CANNOT_FORCE	"No write permission for \"%s\".\n"\
							"Unable to --force conversion, since you are not the owner of the file.\n"\
							"If you have root/sudo access, use chmod to make the file writeable.\n"
#define	EMSG_NOTWRITEABLE_NOT_OWNER	"No write permission for \"%s\".\n"
#define	EMSG_CHOWN			"Unable to change ownership of \"%s\".\n"
#define	EMSG_UTIME			"Unable to preserve file time of \"%s\".\n"
#define	EMSG_SYMLINK		"Unable to dereference symbolic link \"%s\".\n"
#define	EMSG_CREATETEMP		"Unable to create temporary file \"%s\" for converting \"%s\".\n"
#define	EMSG_ERRORLOG		"%s: Unable to create error log file \"%s\". Defaulting to stderr.\n" /* special case with progname */
#define	EMSG_RENAMEBAK		"Unable to save original file %s as %s.\nReason: %s\n"
#define	EMSG_RENAMETMP		"Unable to rename temporary file %s back to %s after converting it.\nReason: %s\n"\
							"Please recover your converted file by manually renaming it back.\n"
#define	EMSG_RENAME_TMP_AS_BAK	"Unable to rename temporary file %s to %s.\nReason: %s\n"\
								"You will have to manually rename it to the backup filename yourself.\n"
#define	EMSG_PATHTOOLONG	"Filename is too long: %s\n"
#define	EMSG_BADFILENAME	"Invalid filename: %s\n"
#define	EMSG_NODIRECTION	"Direction of conversion not specified. Either use --fromdos or --todos, or\n"\
							"rename your program file \"todos\" or \"fromdos\" to set the default direction.\n"
#define	EMSG_WRITETEMP		"Unable to write temporary file \"%s\".\n"
#define	EMSG_READFILE		"Unable to read from \"%s\".\n"
#define	EMSG_OPENTEMP		"Unable to open temporary file \"%s\".\n"
#if defined(WIN32)
#define	EMSG_MAKEWRITEABLE	"Unable to remove read-only attribute from \"%s\".\n"
#else
#define	EMSG_MAKEWRITEABLE	"Unable to make file writeable: \"%s\".\n"
#endif
#define	EMSG_CONVERT_MOREINFO	"The original file, \"%s\", may have been partially modified.\n"\
								"The contents of the original file (before any conversion was attempted) was saved in\n"\
								"the file \"%s\". You may recover its content by copying it back into the original.\n"
#define	EMSG_FILEINFO		"Unable to get file information from the system about \"%s\".\n"

#define	EMSG_DIRNOTWRITEABLE	"Unable to back up %s because the directory it is in is not writeable.\n"\
								"Aborted. Your file was not changed.\n"
#define	EMSG_TMPDIR			"Unable to create file in the temporary directory to convert %s.\n"
#define	EMSG_TOO_MANY_EXT_OR_EXCLUDES	"Too many extensions or excludes specified.\n"
#define	EMSG_EXTREQUIRED	"You need to specify at least one extension to convert for recursive mode.\n"
#define	EMSG_DIR_TOO_DEEP	"Directory is too deep or its name is too long: %s\n"
#define	EMSG_FTS_OPEN		"Unable to open handle to directory.\n"
#define	EMSG_FTS_ERROR		"%s: %s\n"
#define	EMSG_BROKEN_SYMLINK	"Symlink points to a non-existent file or directory: %s.\n"
#define	EMSG_SYMLINK_UNKNOWN	"Symlink does not point to directory or regular file: %s.\n"
#define	EMSG_NOT_RECURSIVE_MODE "Options -i, -x, -X and their long versions are only valid in recursive mode (-r).\n"

/* internal error macros */
#define	EINTNL_DIRECTION	"unknown conversion direction"

/* verbose messages */
#define	VERBOSE_CONVERTING	"Converting \"%s\"\n"
#define	VERBOSE_SYMLINKSRC	"\"%s\" resolves to \"%s\"\n"
#define	VERBOSE_ALTMETHOD	"\"%s\" either has multiple hard links or is in a directory you don't\n"\
							"have write access. Using alternative method to convert.\n"
#define	VERBOSE_RECURSE_CURDIR	"Recursing through current directory.\n"
#define	VERBOSE_RECURSE_DIR		"Recursing through %s\n"
#define	VERBOSE_RECURSE_SKIP_DIR	"Skipping excluded directory: %s\n"
#define	VERBOSE_RECURSE_SKIP_FILE	"Skipping excluded file: %s\n"
#define	VERBOSE_RECURSE_SKIP_UNREADABLE_DIR "Skipping unreadable directory: %s\n"

/* function declarations */
extern void emsg ( char * message, ... );

#if defined(__cplusplus)
}
#endif

#endif
