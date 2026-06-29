@echo off
rem Build MSVC app

if not "%1" == "" goto start
echo Usage: %0 application
pause
goto end

:start
set MAKEFILE=%1.dsp

if not exist %1\msvc6ws2\%MAKEFILE% goto end

echo.
echo Building %1...

rem MSVC setup
if not "%MSDevDir%" == "" goto make

call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat"

:make

cd %1\msvc6ws2

msdev %MAKEFILE% /make all
if errorlevel 1 goto batchend

rmdir /s /q Debug
if exist Release\*.exe move /y Release\*.exe .
rmdir /s /q Release
if exist *.ncb del *.ncb
if exist *.plg del *.plg

goto done

:batchend
pause

:done
cd ..\..

:end
