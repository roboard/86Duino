@echo off
rem Build DJGPP app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start

if not exist %1\gnu\makefile goto end

echo.
echo Building %1...

rem DJGPP setup
if not "%DJGPP%" == "" goto make

set DJDIR=C:\DJGPP
set DJGPP=%DJDIR%\djgpp.env
set PATH=%DJDIR%\bin;%PATH%

:make

cd %1\gnu

rem goto rel
if exist *.exe del *.exe

:@echo on
make -s -k DEBUG=1 VERBOSITY=0 clean all
:@echo off
if errorlevel 1 goto batchend
make DEBUG=1 tidy
ren *.exe dbg.exe

:rel
:@echo on
make -s -k VERBOSITY=0 clean all
:@echo off
if errorlevel 1 goto batchend
make tidy
goto done

:batchend
pause

:done
cd ..\..

:end
