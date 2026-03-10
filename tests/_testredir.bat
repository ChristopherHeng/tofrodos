@echo off
rem _testredir.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem Do not run this file from the command line. It's meant to be called by runtests.bat

setlocal

..\fromdos < crlf\utf8.txt > temp\utf8.txt
diff --binary lf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
echo PASSED: redirection test
del temp\utf8.txt
goto EndScript

:FailedTest
echo FAILED: redirection test
>> temp\test-fails-log.txt echo redirection test
goto EndScript

:EndScript
endlocal
