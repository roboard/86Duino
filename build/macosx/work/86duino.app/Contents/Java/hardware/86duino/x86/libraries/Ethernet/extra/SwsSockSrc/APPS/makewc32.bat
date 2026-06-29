@echo off
rem Build WatcomC app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start

set MAKEFILE=wcdos32.mak

if not exist %1\watcom\%MAKEFILE% goto end

echo.
echo Building %1...

rem Watcom setup
if not "%WATCOM%" == "" goto make

SET WATCOM=C:\WATCOM
SET PATH=%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%
SET EDPATH=%WATCOM%\EDDAT
SET INCLUDE=%WATCOM%\H;%WATCOM%\H\NT
REM SET LIB=

:make

cd %1\watcom

rem goto rel
if exist *.exe del *.exe

:@echo on
wmake -h -s -k -f %MAKEFILE% DEBUG=1 clean all
:@echo off
if errorlevel 1 goto batchend
ren *.exe dbg.exe
rmdir /s /q dbg

:rel
:@echo on
wmake -h -s -k -f %MAKEFILE% NDEBUG=1 clean all
:@echo off
if errorlevel 1 goto batchend
rmdir /s /q rel
if exist *.map del *.map
if exist *.sym del *.sym
goto done

:batchend
pause

:done
cd ..\..

:end
