@echo off
rem _test02.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem Do not run this file from the command line. It's meant to be called by runtests.bat

setlocal

echo Making sure todos does not double-convert a file that already has CRLF
copy crlf\utf8-with-bom.txt temp > nul 2>&1
set TOFRODOS_CMD=..\todos temp\utf8-with-bom.txt
%TOFRODOS_CMD%
diff --binary crlf\utf8-with-bom.txt temp\utf8-with-bom.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%
del temp\utf8-with-bom.txt
goto EndScript

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
goto EndScript

:EndScript
endlocal
