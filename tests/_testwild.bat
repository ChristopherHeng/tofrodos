@echo off
rem _testwild.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem Do not run this file from the command line. It's meant to be called by runtests.bat

setlocal

echo Testing with wildcards
copy crlf\utf8*.txt temp > nul 2>&1
copy crlf\normal.txt temp > nul 2>&1
set TOFRODOS_CMD=..\fromdos temp\u*.txt temp\n*.txt
%TOFRODOS_CMD%
diff --binary lf\utf8-with-bom.txt temp\utf8-with-bom.txt
if errorlevel 1 goto FailedTest
diff --binary lf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
diff --binary lf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%
del temp\utf8-with-bom.txt temp\utf8.txt temp\normal.txt
goto EndScript

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
goto EndScript

:EndScript
endlocal
