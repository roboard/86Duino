@echo off
rem Build app with DJGPP

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
if exist *.exe del *.exe

@echo on
make -s -k DEBUG=1 clean all
@echo off
if errorlevel 1 goto batchend
ren *.exe dbg.exe
make DEBUG=1 tidy

@echo on
make -s -k clean all
@echo off
if errorlevel 1 goto batchend
make tidy

:batchend
pause

:end
