@echo off
rem Build MSVC1 app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start
set MAKEFILE=%1.mak

if not exist %1\msvc1\%MAKEFILE% goto end

echo.
echo Building %1...

rem MSVC1 setup
if not "%TOOLROOTDIR%" == "" goto make

call "C:\MSVC\BIN\MSVCVARS.bat"

:make

cd %1\msvc1

if exist swssock.mak nmake /S /C /A -f swssock.mak DEBUG=0 all

nmake /S /C /A -f %MAKEFILE% DEBUG=0 all
if errorlevel 1 goto batchend

goto done

:batchend
pause

:done
if exist *.lib del *.lib
if exist *.obj del *.obj
if exist *.sbr del *.sbr
if exist *.pdb del *.pdb
if exist *.bsc del *.bsc
cd ..\..

:end
