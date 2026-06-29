@echo off
rem Build with Watcom

set MAKEFILE=swsck16.mak

rem Watcom setup
if not "%WATCOM%" == "" goto make
SET WATCOM=C:\WATCOM
SET PATH=%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%
SET EDPATH=%WATCOM%\EDDAT
SET INCLUDE=%WATCOM%\H;%WATCOM%\H\NT
REM SET LIB=

:make
if "%1" == "" goto batch

@echo on
wmake -f %MAKEFILE% %1 %2 %3 %4 %5 %6 %7 %8 %9
@echo off
goto end


:batch
@echo on
wmake -h -s -k -f %MAKEFILE% DEBUG=1 clean all
@echo off
if errorlevel 1 goto batchend

@echo on
wmake -h -s -k -f %MAKEFILE% NDEBUG=1 clean all
@echo off
if errorlevel 1 goto batchend

:batchend
pause

:end
