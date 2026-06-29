@echo off
if not "%BC%" == "" goto start
set BC=c:\borlandc\bin
path=%BC%;%path%
if "%1" == "/c" goto end

:start
swsvpkt.exe
bc.exe %1 %2 %3 %4 %5 %6 %7 %8 %9
:end
