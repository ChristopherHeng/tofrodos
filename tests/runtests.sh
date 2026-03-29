#!/bin/bash
# runtests.sh
# Copyright 2026 by Christopher Heng. All rights reserved.

# 1. Do not execute this in MSYS2 or Cygwin; it is meant for a real Unix-type system like
# Linux, etc. On Windows, use runtests.bat instead.
#
# 2. Note that the script uses "getent" to test the existence of a group. If your OS
# does not have that program, you may still be able to run this script by using
# the -s option, which skips this test. In such a case, you should also use the
# -u and -g options to specify a user and group that actually exist on your system
# (other than the current owner/group of the test files).
# Type runtests.sh -h (or read the show_usage() function below) for more information.
# I think macOS is in this category.
#
# 3. The script uses "sudo chown" to change ownership of a test file, and then later
# "sudo fromdos" to convert it, in order to test the -p (or --preserve) flag. As such,
# you may be required to furnish a root password in the middle of the test.

set -e -u -o pipefail
# -e exit if any cmd fails (either check it with "if" or append "|| true" to avoid this)
# -u treat unset variables as error when performing parameter expansion
# -o pipefail retain the exit code of last failing command instead of the last command in a pipeline
# Note to self: for debugging the script, you can set -x to echo all commands.

# functions

show_usage()
{
	echo "Usage: runtests.sh [-u user][-g group][-h][-s]"
	echo "Options:"
	echo "-u user - a valid user on your system; must not be the current owner of the test files (default: nobody)"
	echo "-g group - a valid group on your system; must not be the current group that owns the test files (default: nogroup)"
	echo "-h - show help on usage and exit"
	echo "-s - skip test to verify that the user and group are valid on this system"
	exit 0
}

directory_exists()
{
	if [ "$1" = "" ] ; then
		echo "Internal error: missing argument to directory_exists"
		exit 1
	fi
	if [ ! -d "$1" ] ; then
		echo "This script must be executed with tests as the current working directory"
		exit 1
	fi
}

check_make_done_for()
{
	if [ "$1" = "" ] ; then
		echo "Internal error: missing argument to check_make_done_for"
		exit 1
	fi
	if [ ! -f "../$1" ] ; then
		echo "run make -f makefile.gcc all before running this script"
		exit 1
	fi
}

print_standard_fail_message()
{
	echo FAILED: "${tofrodos_cmd}"
	echo "${tofrodos_cmd}" >> temp/test-fails-log.txt
}

convert_and_compare()
{
	# test correct conversion for todos, fromdos, dos2unix, unix2dos

	local src_dir=lf
	local cf_dir=crlf
	local tofrodos_cmd="$1 temp/$2"

	if [ "$1" = "../todos" ] || [ "$1" = "temp/unix2dos" ] ; then
		src_dir=lf
		cf_dir=crlf
	elif [ "$1" = "../fromdos" ] || [ "$1" = "temp/dos2unix" ] ; then
		src_dir=crlf
		cf_dir=lf
	else
		echo Internal error: invalid or missing argument to convert_and_compare
		exit 1
	fi
	cp ${src_dir}/$2 temp/$2
	${tofrodos_cmd} || true
	if diff --binary temp/$2 ${cf_dir}/$2 ; then
		rm -f temp/$2
		echo PASSED: "${tofrodos_cmd}"
	else
		print_standard_fail_message
	fi
}

test_symlink()
{
	# check if we handle symbolic links correctly

	local tofrodos_cmd="../todos temp/symlink_to_normal.txt"

	mkdir temp/subdir
	cp lf/normal.txt temp/subdir
	ln -s $(pwd)/temp/subdir/normal.txt temp/symlink_to_normal.txt
	${tofrodos_cmd} || true
	if diff --binary temp/subdir/normal.txt crlf/normal.txt ; then
		rm temp/symlink_to_normal.txt
		rm temp/subdir/normal.txt
		rmdir temp/subdir
		echo PASSED: "${tofrodos_cmd}"
	else
		print_standard_fail_message
	fi
}

check_if_user_group_exists()
{
	if ! id ${test_user} > /dev/null 2>&1 ; then
		echo "This script assumes that a user named ${test_user} exists on your system so that it can test"
		echo "the --preserve flag. Since it does not exist, you should run this script with:"
		echo "  ./runtests.sh -u username"
		echo "where username is the name of a user other than you (the current user)."
		exit 1
	fi
	if ! getent group ${test_group} > /dev/null 2>&1 ; then
		echo "This script assumes that a group named ${test_group} exists on your system so that it can test"
		echo "the --preserve flag. Since it does not exist, you should run this script with:"
		echo "  ../runtests.sh -g groupname"
		echo "where groupname is the name of a group other than your login group."
		exit 1 ;
	fi
}

test_preserve()
{
	# testing -p (or --preserve)

	local tofrodos_cmd="sudo ../fromdos -p temp/utf8.txt"

	cp crlf/utf8.txt temp
	cp crlf/empty.txt temp
	touch "--date=2004-01-01 5:00:00" temp/utf8.txt temp/empty.txt
	echo "Testing -p. You may be prompted for a password for sudo chown and sudo fromdos"
	chmod 666 temp/utf8.txt
	sudo chown ${test_user}:${test_group} temp/utf8.txt
	${tofrodos_cmd} || true

	local file_user="$(stat -c '%U' temp/utf8.txt)"
	local file_group="$(stat -c '%G' temp/utf8.txt)"
	if [ "${file_user}" = "${test_user}" ] && [ "${file_group}" = "${test_group}" ] && [ ! temp/utf8.txt -nt temp/empty.txt ] ; then
		rm temp/utf8.txt temp/empty.txt
		echo PASSED: "${tofrodos_cmd}"
	else
		print_standard_fail_message
	fi
}

double_conversion()
{
	# make sure todos handles a file that already has CRLF correctly
	cp crlf/utf8.txt temp/double-conversion.txt
	local tofrodos_cmd="../todos temp/double-conversion.txt"
	${tofrodos_cmd} || true
	if diff --binary crlf/utf8.txt temp/double-conversion.txt ; then
		rm temp/double-conversion.txt
		echo PASSED: ${tofrodos_cmd}
	else
		print_standard_fail_message
	fi
}

test_pipe()
{
	# check if we can handle redirected input correctly
	if cat lf/utf8-with-bom.txt | ../todos | diff --binary - crlf/utf8-with-bom.txt ; then
		echo 'PASSED: cat lf/utf8-with-bom.txt | ../todos | diff --binary - crlf/utf8-with-bom.txt'
	else
		echo 'FAILED: cat lf/utf8-with-bom.txt | ../todos | diff --binary - crlf/utf8-with-bom.txt'
		echo 'cat lf/utf8-with-bom.txt | ../todos | diff --binary - crlf/utf8-with-bom.txt' >> temp/test-fails-log.txt
	fi
}

test_read_only()
{
	# check if we handle readonly (not writeable) files correctly, and
	# whether -f (ie, --force) works correctly
	cp crlf/normal.txt temp/not-writeable.txt
	chmod u-w temp/not-writeable.txt
	local tofrodos_cmd="../fromdos temp/not-writeable.txt"
	${tofrodos_cmd} > temp/readonlyerr.txt 2>&1 || true
	if diff expected/readonlyerr-linux.txt temp/readonlyerr.txt ; then
		rm temp/readonlyerr.txt
		echo PASSED: ${tofrodos_cmd}
	else
		print_standard_fail_message
	fi

	tofrodos_cmd="../fromdos -f temp/not-writeable.txt"
	${tofrodos_cmd} > temp/readonlyforce.txt 2>&1 || true
	if diff expected/readonlyforce.txt temp/readonlyforce.txt ; then
		# no errors
		rm temp/readonlyforce.txt

		# did we set the file permissions correctly?
		if [ -w temp/not-writeable.txt ] ; then
			print_standard_fail_message
		else
			rm -f temp/not-writeable.txt
			echo PASSED: ${tofrodos_cmd}
		fi
	else
		# there was an error message
		print_standard_fail_message
	fi
}

test_read_only_non_owner()
{
	# check if we handle a read-only file that we don't own correctly
	cp lf/utf8.txt temp/not-owned-readonly.txt
	chmod o-w temp/not-owned-readonly.txt
	sudo chown ${test_user}:${test_group} temp/not-owned-readonly.txt
	tofrodos_cmd="../todos -l temp/notownederr-linux.txt temp/not-owned-readonly.txt"
	${tofrodos_cmd} || true
	if diff expected/notownederr-linux.txt temp/notownederr-linux.txt ; then
		rm temp/notownederr-linux.txt
		echo PASSED: ${tofrodos_cmd}
	else
		print_standard_fail_message
	fi
	tofrodos_cmd="../todos -l temp/notownedforceerr-linux.txt --force temp/not-owned-readonly.txt"
	${tofrodos_cmd} || true
	if diff expected/notownedforceerr-linux.txt temp/notownedforceerr-linux.txt ; then
		rm -f temp/notownedforceerr-linux.txt temp/not-owned-readonly.txt
		echo PASSED: ${tofrodos_cmd}
	else
		print_standard_fail_message
	fi
}

test_backup()
{
	# test -b (or --backup)
	cp crlf/utf8.txt temp
	local tofrodos_cmd="../fromdos -b temp/utf8.txt"
	${tofrodos_cmd} || true
	if diff --binary crlf/utf8.txt temp/utf8.txt.bak ; then
		echo PASSED: ${tofrodos_cmd}
		rm temp/utf8.txt temp/utf8.txt.bak
	else
		print_standard_fail_message
	fi
}

test_exitonerror()
{
	# test -e (or --exit-on-error) together with -l (or --log)
	cp crlf/normal.txt temp
	cp crlf/one-newline.txt temp
	local tofrodos_cmd="../fromdos -e -l temp/logemsg.txt temp/normal.txt temp/no-such-file.txt temp/one-newline.txt"
	${tofrodos_cmd} || true
	if diff --binary lf/normal.txt temp/normal.txt ; then
		if [ -f temp/no-such-file.txt  ] ; then
			print_standard_fail_message
		else
			if diff expected/logemsg-linux.txt temp/logemsg.txt ; then
				if diff --binary temp/one-newline.txt crlf/one-newline.txt ; then
					rm temp/logemsg.txt temp/one-newline.txt temp/normal.txt
					echo PASSED: ${tofrodos_cmd}
				else
					# it should have remained unconverted
					print_standard_fail_message
				fi
			else
				print_standard_fail_message
			fi
		fi
	else
		print_standard_fail_message
	fi
}

test_badfn_in_list()
{
	# test a list of filenames with one non-existent one in the middle to make sure we
	# skip to the next one; also tests -l
	cp crlf/normal.txt temp
	cp crlf/one-newline.txt temp
	local tofrodos_cmd="../fromdos -l temp/logemsg.txt temp/normal.txt temp/no-such-file.txt temp/one-newline.txt"
	${tofrodos_cmd} || true
	if diff --binary lf/normal.txt temp/normal.txt ; then
		if diff expected/logemsg-linux.txt temp/logemsg.txt ; then
			if diff --binary temp/one-newline.txt lf/one-newline.txt ; then
				rm temp/logemsg.txt temp/one-newline.txt temp/normal.txt
				echo PASSED: ${tofrodos_cmd}
				return
			fi
		fi
	fi
	print_standard_fail_message
}

test_non_standard_name()
{
	local tofrodos_cmd="temp/tofrodos -l temp/nonstdname.txt temp/utf8.txt"
	cp lf/utf8.txt temp
	${tofrodos_cmd} || true
	if diff temp/nonstdname.txt expected/nonstdname.txt ; then
		if diff --binary temp/utf8.txt lf/utf8.txt ; then
			rm temp/nonstdname.txt temp/utf8.txt
			echo PASSED: "${tofrodos_cmd}"
			return
		fi
	fi
	print_standard_fail_message
}

test_hard_links()
{
	# this function is really ugly, but I cannot be bothered to do it properly;
	# it is practically a transcription of my manual tests

	local tofrodos_cmd="../fromdos temp/subdir/altname.txt"

	cp crlf/utf8.txt temp
	mkdir temp/subdir
	ln temp/utf8.txt temp/subdir/altname.txt
	${tofrodos_cmd} || true
	if diff lf/utf8.txt temp/subdir/altname.txt ; then
		if diff lf/utf8.txt temp/utf8.txt ; then
			echo PASSED: "${tofrodos_cmd}"
			chmod a-w temp/utf8.txt
			tofrodos_cmd="../todos -l temp/nowritemulti-linux.txt temp/subdir/altname.txt"
			${tofrodos_cmd} || true ;
			if diff temp/nowritemulti-linux.txt expected/nowritemulti-linux.txt ; then
				rm temp/nowritemulti-linux.txt
				echo PASSED: "${tofrodos_cmd}"
				tofrodos_cmd="../todos --force temp/subdir/altname.txt"
				${tofrodos_cmd} || true ;
				if diff crlf/utf8.txt temp/utf8.txt ; then
					echo PASSED: "${tofrodos_cmd}"
					sudo chown ${test_user}:${test_group} temp/utf8.txt
					tofrodos_cmd="../fromdos -l temp/notownermulti-linux.txt temp/subdir/altname.txt"
					${tofrodos_cmd} || true
					if diff temp/notownermulti-linux.txt expected/notownermulti-linux.txt ; then
						rm temp/notownermulti-linux.txt
						echo PASSED: "${tofrodos_cmd}"
						tofrodos_cmd="../fromdos --force -l temp/forcenowritefail-multi-linux.txt temp/subdir/altname.txt"
						${tofrodos_cmd} || true
						if diff temp/forcenowritefail-multi-linux.txt expected/forcenowritefail-multi-linux.txt ; then
							rm temp/forcenowritefail-multi-linux.txt
							echo PASSED: "${tofrodos_cmd}"
							sudo chmod 666 temp/utf8.txt
							tofrodos_cmd="../fromdos temp/subdir/altname.txt"
							${tofrodos_cmd} || true
							if diff lf/utf8.txt temp/utf8.txt ; then
								echo PASSED: "${tofrodos_cmd}"
								sudo touch "--date=2004-01-01 5:00:00" temp/utf8.txt
								tofrodos_cmd="../todos -l temp/preservemulti-linux.txt --preserve temp/subdir/altname.txt"
								${tofrodos_cmd} || true
								if diff temp/preservemulti-linux.txt expected/preservemulti-linux.txt ; then
									# Do not bother to check the file time. It will have failed since we are not the owner of the file.
									# The error message file above will have shown it.
									rm temp/preservemulti-linux.txt
									echo PASSED: "${tofrodos_cmd}"
									sudo touch "--date=2004-01-01 5:00:00" temp/utf8.txt temp/empty.txt
									tofrodos_cmd="../fromdos --backup temp/utf8.txt"
									${tofrodos_cmd} || true
									# the backup file will have the file time preserved though not the owner
									if [ ! temp/utf8.txt.bak -nt temp/empty.txt ] ; then
										if diff lf/utf8.txt temp/subdir/altname.txt ; then
											if diff crlf/utf8.txt temp/utf8.txt.bak ; then
												echo PASSED: "${tofrodos_cmd}"
												rm -f temp/empty.txt temp/utf8.txt.bak temp/utf8.txt temp/subdir/altname.txt
												rmdir temp/subdir
												return
											fi
										fi
									fi
								fi
							fi
						fi
					fi
				fi
			fi
		fi
	fi
	print_standard_fail_message
}

# start of script-proper

test_user=nobody
test_group=nogroup
skip_user_group_check=0

directory_exists temp
directory_exists crlf
directory_exists lf
directory_exists expected
check_make_done_for fromdos
check_make_done_for todos

while getopts "u:g:h" opt ; do
	case "${opt}" in
		u) test_user=${OPTARG} ;;
		g) test_group=${OPTARG} ;;
		h) show_usage ;;
		s) skip_user_group_check=1 ;;
	esac
done
if [ ${skip_user_group_check} = 0 ]; then
	check_if_user_group_exists
fi

if [ -f temp/test-fails-log.txt ] ; then
	mv temp/test-fails-log.txt temp/test-fails-log.txt.bak
fi

for fname in crlf/*.txt ; do convert_and_compare ../fromdos "${fname##*/}" ; done
for fname in lf/*.txt ; do convert_and_compare ../todos "${fname##*/}" ; done
cp ../fromdos temp/dos2unix
cp ../fromdos temp/unix2dos
convert_and_compare temp/dos2unix normal.txt
convert_and_compare temp/unix2dos normal.txt
rm temp/dos2unix temp/unix2dos
test_symlink
test_preserve
test_read_only
test_read_only_non_owner
test_backup
test_exitonerror
test_badfn_in_list
double_conversion
test_pipe

cp ../todos temp/tofrodos
test_non_standard_name
rm temp/tofrodos

test_hard_links

if [ -f temp/test-fails-log.txt ] ; then
	echo One or more tests FAILED. See temp/test-fails-log.txt.
	exit 1
else
	echo All tests PASSED.
	exit 0
fi
