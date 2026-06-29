@echo off
rem Build ECHOS for BorlandC 3.1

set MAKEFILE=echos.prj

rem BorlandC setup
if not "%BC%" == "" goto make
set BC=c:\borlandc\bin
set path=%BC%;%path%

:make
@echo on
bc.exe /b %MAKEFILE%
@echo off
if errorlevel 1 goto batchend

@del *.obj

:batchend
pause

:end
