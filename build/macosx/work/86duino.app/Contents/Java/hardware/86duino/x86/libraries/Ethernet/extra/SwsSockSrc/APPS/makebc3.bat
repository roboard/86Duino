@echo off
rem Build BorlandC 3.1 app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start
set MAKEFILE=%1.prj

if not exist %1\bc3\%MAKEFILE% goto end

echo.
echo Building %1...

rem BC3 setup
if not "%BC%" == "" goto make
set BC=c:\borlandc\bin
set path=%BC%;%path%

:make

cd %1\bc3

bc.exe /b %MAKEFILE%
if errorlevel 1 goto batchend

goto done

:batchend
pause

:done
if exist *.obj del *.obj
cd ..\..

:end
