@echo off
rem _preprecur.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.
rem Create directory structure and files for --recursive test.

rem Do not run directly. Use runtests.bat

rem if %1 == dos, will only prepare the subset needed for testing on MSDOS

rem Construct directory for recursive tests
md temp\recur > nul 2>&1
md temp\recur\lib > nul 2>&1
md temp\recur\windows > nul 2>&1
md temp\recur\posix > nul 2>&1
md temp\recur\posix\linux > nul 2>&1
md temp\recur\posix\macos > nul 2>&1
md temp\recur\posix\bsd > nul 2>&1
md temp\recur\dos > nul 2>&1
md temp\recur\private > nul 2>&1
md temp\recur\secret > nul 2>&1
md temp\recur\secret\top > nul 2>&1

if "%1" == "dos" goto SkipMDOutside
md temp\outside > nul 2>&1
md temp\outside\others > nul 2>&1

:SkipMDOutside
copy lf\utf8.txt temp\recur\abc.c > nul 2>&1
copy lf\utf8.txt temp\recur\def.c > nul 2>&1
copy lf\utf8.txt temp\recur\ghi.c > nul 2>&1
copy lf\utf8.txt temp\recur\abc.h > nul 2>&1
copy lf\utf8.txt temp\recur\etc.h > nul 2>&1
copy lf\utf8.txt temp\recur\readme.txt > nul 2>&1

if "%1" == "dos" goto CopyNOCONVRT.TXT
copy lf\utf8.txt temp\recur\dontconvert.txt > nul 2>&1
goto ResumeCommon
:CopyNOCONVRT.TXT
copy lf\utf8.txt temp\recur\NOCONVRT.TXT > nul 2>&1

:ResumeCommon
copy lf\utf8.txt temp\recur\lib\some.c > nul 2>&1
copy lf\utf8.txt temp\recur\lib\file.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\p1.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\p2.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\ph.h > nul 2>&1
copy lf\utf8.txt temp\recur\posix\linux\lx.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\linux\g.h > nul 2>&1
copy lf\utf8.txt temp\recur\posix\macos\mac.md > nul 2>&1
copy lf\utf8.txt temp\recur\posix\bsd\free.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\bsd\open.c > nul 2>&1
copy lf\utf8.txt temp\recur\posix\bsd\net.h > nul 2>&1
copy lf\utf8.txt temp\recur\dos\msver.bas > nul 2>&1
copy lf\utf8.txt temp\recur\dos\dosbox.bas > nul 2>&1
copy lf\utf8.txt temp\recur\dos\freedos.c > nul 2>&1
copy lf\utf8.txt temp\recur\private\special.c > nul 2>&1
copy lf\utf8.txt temp\recur\secret\knowledge.c > nul 2>&1
copy lf\utf8.txt temp\recur\secret\top\tricks.txt > nul 2>&1
copy lf\utf8.txt temp\recur\windows\w1.c  > nul 2>&1

if "%1" == "dos" goto Finished
copy lf\utf8.txt temp\outside\general.c > nul 2>&1
copy lf\utf8.txt temp\outside\others\readme.md > nul 2>&1
powershell Start-Process cmd -ArgumentList "/c","mklink","/D","%~dp0\temp\recur\manuals","%~dp0\temp\outside\others" -Verb RunAs -Wait
if not exist temp\recur\manuals goto DirSymlinkFailed
powershell Start-Process cmd -ArgumentList "/c","mklink","%~dp0\temp\recur\lib\gen.c","%~dp0\temp\outside\general.c" -Verb RunAs -Wait
if not exist temp\recur\lib\gen.c goto FileSymlinkFailed

:Finished
exit /B 0

:DirSymlinkFailed
echo Unable to make a symlink temp\recur\manuals to temp\outside\others
>> temp\test-fails-log.txt echo Unable to make a symlink temp\recur\manuals to temp\outside\others
exit /B 1

:FileSymlinkFailed
echo Unable to make a symlink temp\recur\lib\gen.c to temp\outside\general.c
>> temp\test-fails-log.txt echo Unable to make a symlink temp\recur\lib\gen.c to temp\outside\general.c
exit /B 1

