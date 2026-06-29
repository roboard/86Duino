@echo off
rem Build with DJGPP

rem DJGPP setup
if not "%DJGPP%" == "" goto make

set DJDIR=C:\DJGPP
set DJGPP=%DJDIR%\djgpp.env
set PATH=%DJDIR%\bin;%PATH%

:make
if "%1" == "" goto batch

@echo on
make %1 %2 %3 %4 %5 %6 %7 %8 %9
@echo off
goto end


:batch

@echo on
make -k -s DEBUG=1 clean all
@echo off
if errorlevel 1 goto batchend
make DEBUG=1 tidy

:rel
@echo on
make -k -s clean all
@echo off
if errorlevel 1 goto batchend
make tidy

:batchend
pause

:end
