@echo off
rem Build app with CYGWIN

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
if exist *.exe del *.exe

@echo on
bash -c "make -s -k DEBUG=1 clean all"
@echo off
if errorlevel 1 goto batchend
ren *.exe dbg.exe
bash -c "make DEBUG=1 tidy"

@echo on
bash -c "make -s -k clean all"
@echo off
if errorlevel 1 goto batchend
bash -c "make tidy"

:batchend
pause

:end
