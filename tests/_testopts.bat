@echo off
rem _testopts.bat
rem Copyright 2026 by Christopher Heng. All rights reserved.

rem This batch file requires Windows' CMD.EXE.

setlocal

echo Testing with option flags

rem Test on readonly file, with/without -f
copy lf\normal.txt temp > nul 2>&1
attrib +r temp\normal.txt
set TOFRODOS_CMD=..\todos temp\normal.txt
%TOFRODOS_CMD% > temp\readonlyerr.txt 2>&1
set NEXT_TEST_LABEL=TestWithOptf
diff expected\readonlyerr.txt temp\readonlyerr.txt
if errorlevel 1 goto FailedTest
echo PASSED: %TOFRODOS_CMD% (with read-only attribute)
del temp\readonlyerr.txt > nul 2>&1

:TestWithOptf
set TOFRODOS_CMD=..\todos -f temp\normal.txt
%TOFRODOS_CMD% > temp\readonlyforce.txt 2>&1
set NEXT_TEST_LABEL=CheckROAttrib
diff expected\readonlyforce.txt temp\readonlyforce.txt
if errorlevel 1 goto FailedTest
del temp\readonlyforce.txt > nul 2>&1

:CheckROAttrib
for %%i in (temp\normal.txt) do echo %%~ai > temp\readonlyattrib.txt
set NEXT_TEST_LABEL=
diff expected\readonlyattrib.txt temp\readonlyattrib.txt
if errorlevel 1 goto AttribNotPreserved
del /f temp\normal.txt > nul 2>&1
del temp\readonlyattrib.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD% (with read-only attribute)

:TestOptb
rem Test -b
copy crlf\utf8.txt temp > nul 2>&1
set TOFRODOS_CMD=..\fromdos -b temp\utf8.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptd
diff --binary crlf\utf8.txt temp\utf8.txt.bak
if errorlevel 1 goto FailedTest
del temp\utf8.txt temp\utf8.txt.bak > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptd
rem Test todos -d (ie, use fromdos/dos2unix mode)
copy crlf\utf8.txt temp > nul 2>&1
set TOFRODOS_CMD=..\todos -d temp\utf8.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptu
diff --binary lf\utf8.txt temp\utf8.txt
if errorlevel 1 goto FailedTest
del temp\utf8.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptu
rem Test fromdos -u (ie use todos/unix2dos mode)
copy lf\utf8-with-bom.txt temp > nul 2>&1
set TOFRODOS_CMD=..\fromdos -u temp\utf8-with-bom.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptel
diff --binary crlf\utf8-with-bom.txt temp\utf8-with-bom.txt
if errorlevel 1 goto FailedTest
del temp\utf8-with-bom.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptel
rem Test -e -l
copy crlf\normal.txt temp > nul 2>&1
copy crlf\one-newline.txt temp > nul 2>&1
set TOFRODOS_CMD=..\fromdos -e -l temp\logemsg.txt temp\normal.txt temp\no-such-file.txt temp\one-newline.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptl
diff --binary lf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
if exist temp\no-such-file.txt goto FailedTest
diff --binary crlf\one-newline.txt temp\one-newline.txt
if errorlevel 1 goto FailedTest
diff expected\logemsg.txt temp\logemsg.txt
if errorlevel 1 goto FailedTest
del temp\normal.txt temp\one-newline.txt temp\logemsg.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptl
rem Test -l alone to make sure that we skip to the next file without -e
copy crlf\normal.txt temp > nul 2>&1
copy crlf\one-newline.txt temp > nul 2>&1
set TOFRODOS_CMD=..\fromdos -l temp\logemsg.txt temp\normal.txt temp\no-such-file.txt temp\one-newline.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptbo
diff --binary lf\normal.txt temp\normal.txt
if errorlevel 1 goto FailedTest
diff --binary lf\one-newline.txt temp\one-newline.txt
if errorlevel 1 goto FailedTest
diff expected\logemsg.txt temp\logemsg.txt
if errorlevel 1 goto FailedTest
del temp\normal.txt temp\one-newline.txt temp\logemsg.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptbo
rem Test -b -o (essentially test -o, since it comes after -b and will be the option that sticks)
rem (It's the only way to test -o since it's the default. It serves this purpose, to overrule an earlier -b.)
copy lf\utf8.txt temp > nul 2>&1
set TOFRODOS_CMD=..\todos -b -o temp\utf8.txt
%TOFRODOS_CMD%
set NEXT_TEST_LABEL=TestOptp
if exist temp\utf8.txt.bak goto FailedTest
del temp\utf8.txt > nul 2>&1
echo PASSED: %TOFRODOS_CMD%

:TestOptp
rem Test -p
copy crlf\utf8-with-bom.txt temp > nul 2>&1
touch -d "2013-01-01 06:00:00Z" temp\utf8-with-bom.txt
set TOFRODOS_CMD=..\fromdos -p temp\utf8-with-bom.txt
%TOFRODOS_CMD%
..\print_file_times temp\utf8-with-bom.txt > temp\filetime.txt 2>&1
set NEXT_TEST_LABEL=
diff --ignore-case expected\filetime.txt temp\filetime.txt
if errorlevel 1 goto FailedTest
del temp\utf8-with-bom.txt temp\filetime.txt
echo PASSED: %TOFRODOS_CMD%

goto EndScript

:AttribNotPreserved
echo FAILED: read-only attribute not preserved for: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo FAILED: read-only attribute not preserved for: %TOFRODOS_CMD%
goto TestOptb

:FailedTest
echo FAILED: %TOFRODOS_CMD%
>> temp\test-fails-log.txt echo %TOFRODOS_CMD%
if not "%NEXT_TEST_LABEL%" == "" goto %NEXT_TEST_LABEL%
goto EndScript

:EndScript
endlocal
