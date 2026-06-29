@echo off
rem Build CYGWIN app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start

if not exist %1\gnu\makefile goto end

echo.
echo Building %1...

rem CYGWIN setup
if not "%CYGDIR%" == "" goto make

set CYGDIR=C:\CYGWIN
set PATH=%CYGDIR%\bin;%PATH%

:make

cd %1\gnu

rem goto rel
if exist *.exe del *.exe

:@echo on
bash -c "make -s -k DEBUG=1 VERBOSITY=0 clean all"
:@echo off
if errorlevel 1 goto batchend
ren *.exe dbg.exe
bash -c "make DEBUG=1 tidy"

:rel
:@echo on
bash -c "make -s -k VERBOSITY=0 clean all"
:@echo off
if errorlevel 1 goto batchend
bash -c "make tidy"
goto done

:batchend
pause

:done
cd ..\..

:end
