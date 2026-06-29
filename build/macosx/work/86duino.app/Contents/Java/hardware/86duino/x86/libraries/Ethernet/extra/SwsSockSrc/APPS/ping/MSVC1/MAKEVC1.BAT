@echo off
rem Build app with MSVC 1.52c

set MAKEFILE=ping.mak

rem MSVC1 setup
if not "%MSVC1%" == "" goto make
set MSVC1=c:\msvc\bin
set path=%MSVC1%;%path%
call MSVCVARS.BAT

:make
if "%1" == "" goto batch

@echo on
make -f %MAKEFILE% %1 %2 %3 %4 %5 %6 %7 %8 %9
@echo off
goto end

:batch
@echo on
nmake /S /C /A -f %MAKEFILE% DEBUG=0 all
@echo off
if errorlevel 1 goto batchend

:batchend
if exist *.obj del *.obj
if exist *.sbr del *.sbr
if exist *.pdb del *.pdb
if exist *.bsc del *.bsc
pause

:end
