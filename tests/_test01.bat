@echo off
rem _test01.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem Do not run this file from the command line. It's meant to be called by runtests.bat

rem usage: _test01 <todos|fromdos> <file to convert>

setlocal
if "%1" == "todos" set CF_DIR=crlf
if "%1" == "fromdos" set CF_DIR=lf
if "%1" == "todos" set SRC_DIR=lf
if "%1" == "fromdos" set SRC_DIR=crlf
if "%CF_DIR%" == "" goto CalledDirectly

copy %SRC_DIR%\%2 temp\%2 > nul 2>&1
set TOFRODOS_CMD=..\%1 temp\%2

%TOFRODOS_CMD%
diff --binary temp\%2 %CF_DIR%\%2
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%
del temp\%2 > nul 2>&1
goto EndScript

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
goto EndScript

:CalledDirectly
echo Do not run this script directly. Use runtests.bat instead.
goto EndScript

:EndScript
endlocal
