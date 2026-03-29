@echo off
rem _testlinks.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.
rem Test symbolic and hard links on Windows

rem Do not run this directly. Use runtests.bat.

setlocal
echo Testing symlinks and hard links
copy crlf\utf8.txt temp > nul 2>&1
powershell Start-Process cmd -ArgumentList "/c","mklink","%~dp0\temp\symfilelink.txt","%~dp0\temp\utf8.txt" -Verb RunAs -Wait
if not exist temp\symfilelink.txt goto CannotMakeSymLink
set TOFRODOS_CMD=..\fromdos temp\symfilelink.txt
set NEXT_TEST_LABEL=HardLinksStart
%TOFRODOS_CMD%
diff --binary lf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD%

rem temp\utf8.txt still exists and is now in LF format
rem temp\symfilelink.txt still points to temp\utf8.txt
:HardLinksStart
md temp\subdir
powershell Start-Process cmd -ArgumentList "/c","mklink","/H","%~dp0\temp\subdir\altname.txt","%~dp0\temp\utf8.txt" -Verb RunAs -Wait
if not exist temp\subdir\altname.txt goto CannotMakeHardLink
set TOFRODOS_CMD=..\todos --backup temp\subdir\altname.txt
set NEXT_TEST_LABEL=SymToHardLinks
%TOFRODOS_CMD%
rem Note to self: the only way to be sure is to use diff; a dir may show old information (probably because Windows caches directory info)
diff --binary crlf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
diff --binary lf\utf8.txt temp\subdir\altname.txt.bak
if errorlevel 1 goto FailedTest
del temp\subdir\altname.txt.bak > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

rem temp\utf8.txt still exists and is now in CRLF format
rem temp\symfilelink.txt still points to temp\utf8.txt
:SymToHardLinks
set TOFRODOS_CMD=..\fromdos --backup temp\symfilelink.txt
set NEXT_TEST_LABEL=
%TOFRODOS_CMD%
diff --binary lf\utf8.txt temp\subdir\altname.txt
if errorlevel 1 goto FailedTest
diff --binary crlf\utf8.txt temp\utf8.txt.bak
if errorlevel 1 goto FailedTest
del temp\subdir\altname.txt temp\utf8.txt temp\utf8.txt.bak temp\symfilelink.txt > nul 2>&1
rd temp\subdir
echo PASSED: %TOFRODOS_CMD%

goto EndScript

:CannotMakeSymLink
echo Unable to make symbolic link temp\symfilelink.txt
>> temp\test-fails-log.txt echo Unable to make symbolic link temp\symfilelink.txt
goto EndScript

:CannotMakeHardLink
echo Unable to make hard link temp\subdir\altname.txt
>> temp\test-fails-log.txt echo Unable to make hard link temp\subdir\altname.txt
goto EndScript

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
if not "%NEXT_TEST_LABEL%" == "" goto %NEXT_TEST_LABEL%
goto EndScript

:EndScript
endlocal
