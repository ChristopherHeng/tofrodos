@echo off
rem _testrecur.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.
rem Tests --recursive

rem Do not run directly. Use runtests.bat

setlocal

rem Create the dir structure and files for the test below.
echo Testing recursion
call _preprecur.bat
if errorlevel 1 goto EndScript

set RECUR_ERROR_OCCURRED=
set TOFRODOS_CMD=..\todos -r -ic -ih -itxt -imd -Xprivate -Xsecret -xdontconvert.txt temp\recur
%TOFRODOS_CMD%

call :CompareFile temp\recur\abc.c
call :CompareFile temp\recur\def.c
call :CompareFile temp\recur\ghi.c
call :CompareFile temp\recur\abc.h
call :CompareFile temp\recur\etc.h
call :CompareFile temp\recur\readme.txt
call :CompareUnconverted temp\recur\dontconvert.txt

call :CompareFile temp\recur\lib\some.c
call :CompareFile temp\recur\lib\file.c
call :CompareFile temp\outside\general.c
call :CompareFile temp\recur\posix\p1.c
call :CompareFile temp\recur\posix\p2.c
call :CompareFile temp\recur\posix\ph.h
call :CompareFile temp\recur\posix\linux\lx.c
call :CompareFile temp\recur\posix\linux\g.h
call :CompareFile temp\recur\posix\macos\mac.md
call :CompareFile temp\recur\posix\bsd\free.c
call :CompareFile temp\recur\posix\bsd\open.c
call :CompareFile temp\recur\posix\bsd\net.h

call :CompareUnconverted temp\recur\dos\msver.bas
call :CompareUnconverted temp\recur\dos\dosbox.bas

call :CompareFile temp\recur\dos\freedos.c

call :CompareUnconverted temp\recur\private\special.c
call :CompareUnconverted temp\recur\secret\knowledge.c
call :CompareUnconverted temp\recur\secret\top\tricks.txt

call :CompareFile temp\recur\windows\w1.c
call :CompareFile temp\outside\others\readme.md

if "%RECUR_ERROR_OCCURRED%" == "1" goto EndScript
echo PASSED: %TOFRODOS_CMD%
rd temp\recur /s /q > nul 2>&1
rd temp\outside /s /q > nul 2>&1

goto EndScript

rem CompareFile is a subroutine; "goto :EOF" or "exit" returns to caller
:CompareFile
set FAILED_FILE=%*
diff --binary crlf\utf8.txt %FAILED_FILE%
if errorlevel 1 goto FailedTest
goto :EOF

rem CompareUnconverted is a subroutine; "goto :EOF" or "exit" returns to caller
:CompareUnconverted
set FAILED_FILE=%1
diff --binary lf\utf8.txt %FAILED_FILE%
if errorlevel 1 goto FailedTest
goto :EOF

:FailedTest
echo FAILED: %TOFRODOS_CMD%
echo Failed at %FAILED_FILE%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo Failed at %FAILED_FILE%
rem Return to caller. Unfortunately I don't know how to break to :EndScript.
rem goto :EndScript does not work the way you think, it just does the endlocal then exits to caller.
set RECUR_ERROR_OCCURRED=1
goto :EOF

:EndScript
endlocal
