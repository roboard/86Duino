@echo off
rem Build SwsSock Library for CYGWIN

rem CYGWIN setup
if not "%CYGDIR%" == "" goto make

set CYGDIR=C:\CYGWIN
set PATH=%CYGDIR%\bin;%PATH%

:make
if "%1" == "" goto batch

@echo on
bash -c "make %1 %2 %3 %4 %5 %6 %7 %8 %9"
@echo off
goto end


:batch
@echo on
bash -c "make -k -s DEBUG=1 clean all"
@echo off
if errorlevel 1 goto batchend
bash -c "make DEBUG=1 tidy"

@echo on
bash -c "make -k -s clean all"
@echo off
if errorlevel 1 goto batchend
bash -c "make tidy"

:batchend
pause

:end
