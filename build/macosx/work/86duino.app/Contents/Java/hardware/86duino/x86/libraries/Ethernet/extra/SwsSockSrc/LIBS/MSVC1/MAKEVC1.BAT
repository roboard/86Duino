@echo off
rem Build SwsSock library for MSVC 1.52c

set TARGET=SWSSOCK
set MAKEFILE=%TARGET%.mak

rem MSVC1 setup
if not "%TOOLROOTDIR%" == "" goto make
call c:\msvc\bin\MSVCVARS.BAT

:make
if "%1" == "" goto batch

@echo on
make -f %MAKEFILE% %1 %2 %3 %4 %5 %6 %7 %8 %9
@echo off
goto end

:batch
if exist *.lib del *.lib

rem goto rel

@echo on
nmake /S /C /A -f %MAKEFILE% DEBUG=1 all
@echo off
if errorlevel 1 goto batchend
rem goto batchend
ren %TARGET%.lib %TARGET%D.LIB

:rel
@echo on
nmake /S /C /A -f %MAKEFILE% DEBUG=0 all
@echo off
if errorlevel 1 goto batchend

:batchend
if exist *.obj del *.obj
if exist *.sbr del *.sbr
if exist *.pdb del *.pdb
if exist *.bsc del *.bsc
if exist *.cod del *.cod
pause

:end
