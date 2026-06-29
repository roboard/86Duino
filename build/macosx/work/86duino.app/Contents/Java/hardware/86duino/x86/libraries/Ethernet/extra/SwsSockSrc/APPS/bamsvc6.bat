@echo off
rem Build all MSVC apps

for /D %%i in (*) do call makevcs.bat %%i
pause
