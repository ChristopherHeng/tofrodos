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

/* internal error macros */
#define	EINTNL_DIRECTION	"unknown conversion direction"

/* verbose messages */
#define	VERBOSE_CONVERTING	"Converting \"%s\"\n"
#define	VERBOSE_SYMLINKSRC	"\"%s\" resolves to \"%s\"\n"
#define	VERBOSE_HARDLINKS	"\"%s\" has multiple hard links. Using alternative method to convert.\n"

/* function declarations */
extern void emsg ( char * message, ... );

#if defined(__cplusplus)
}
#endif

#endif
