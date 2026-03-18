@echo off
rem _testnames.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem Do not run this file from the command line. It's meant to be called by runtests.bat

setlocal

echo Testing with alternate names unix2dos, dos2unix and tofrodos

copy ..\todos.exe temp\unix2dos.exe > nul 2>&1
copy lf\normal.txt temp > nul 2>&1
set TOFRODOS_CMD=temp\unix2dos temp\normal.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=Dos2Unix
diff --binary crlf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%

:Dos2Unix
ren temp\unix2dos.exe dos2unix.exe > nul 2>&1
set TOFRODOS_CMD=temp\dos2unix temp\normal.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=Tofrodos
diff --binary lf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%

:Tofrodos
ren temp\dos2unix.exe tofrodos.exe > nul 2>&1
set TOFRODOS_CMD=temp\tofrodos -l temp\nonstdname.txt temp\normal.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=ForwardSlash
diff expected\nonstdname.txt temp\nonstdname.txt
if errorlevel 1 goto FailedTest
diff --binary lf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%
del temp\normal.txt temp\nonstdname.txt > nul 2>&1

:ForwardSlash
copy lf\utf8.txt temp > nul 2>&1
set TOFRODOS_CMD=temp\tofrodos --todos temp/utf8.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=NoPathPrefix
diff --binary crlf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
del temp\tofrodos.exe temp\utf8.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:NoPathPrefix
copy crlf\utf8-with-bom.txt temp > nul 2>&1
move temp\tofrodos.exe temp\fromdos.exe > nul 2>&1
set TOFRODOS_CMD=fromdos utf8-with-bom.txt
cd temp
%TOFRODOS_CMD%
cd ..
set NEXT_TEST_LABEL=
diff --binary lf\utf8-with-bom.txt temp\utf8-with-bom.txt
if errorlevel 1 goto FailedTest
del temp\fromdos.exe temp\utf8-with-bom.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

goto EndScript

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
if not "%NEXT_TEST_LABEL%" == "" goto %NEXT_TEST_LABEL%
goto EndScript

:EndScript
endlocal
