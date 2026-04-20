@echo off
rem runtests.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.
rem - Run Tests for Windows

rem It's best to run this by typing:
rem     nmake -f makefile.vs test
rem since it will also build set_test_filetime.exe needed for one of the tests.

rem Usage: runtests all
rem The required "all" parameter is so that I won't accidentally run this
rem batch file by selecting Open in the right click context menu instead of Edit.

rem Requirements:
rem 1. This batch file requires CMD.EXE from a modern version of Windows,
rem since it uses things like "setlocal", the substitution of "for" variable
rem references, etc.
rem 2. GNU diff is required.

setlocal

rem Make sure that diff and touch are available
where /Q diff
if "%ERRORLEVEL%" NEQ "0" goto NeedDiff
where /Q touch
if "%ERRORLEVEL%" NEQ "0" goto NeedTouch

if not exist temp goto WrongDirectory
if not exist crlf goto WrongDirectory
if not exist lf goto WrongDirectory
if not exist expected goto WrongDirectory
if not exist ..\todos.exe goto MustBuildFirst
if not exist ..\fromdos.exe goto MustBuildFirst
if not exist ..\print_file_times.exe goto MustBuildTools

if "%1" == "all" goto StartTests
goto ShowUsage

:StartTests

rem Init the error log
if exist temp\test-fails-log.txt move temp\test-fails-log.txt temp\test-fails-log.txt.bak

echo Checking fromdos (dos2unix)
for %%i in (crlf\*.txt) do call _test01.bat fromdos %%~nxi 

echo Checking todos (unix2dos)
for %%i in (lf\*.txt) do call _test01.bat todos %%~nxi

call _test02.bat
call _testwild.bat
call _testopts.bat
call _testnames.bat
call _testredir.bat
call _testlinks.bat

rem Check if any of the tests failed.
if exist temp\test-fails-log.txt goto FailedTest
echo All tests PASSED.
goto EndScript

:FailedTest
echo *** One or more tests FAILED. See temp\test-fails-log.txt
goto EndScript

:NeedDiff
echo This script requires diff (from GNU diffutils).
goto EndScript

:NeedTouch
echo This script requires touch. A Windows version can be obtained from
echo https://github.com/ChristopherHeng/touch/releases
goto EndScript

:WrongDirectory
echo This script must be executed with "tests" as the current working directory.
goto EndScript

:MustBuildFirst
echo Build todos.exe and fromdos.exe before running this script.
goto EndScript

:MustBuildTools
echo print_file_times.exe must first be built, eg with nmake -f makefile.vs print_file_times.exe
goto EndScript

:ShowUsage
echo Usage: runtests all
goto EndScript

:EndScript
endlocal
