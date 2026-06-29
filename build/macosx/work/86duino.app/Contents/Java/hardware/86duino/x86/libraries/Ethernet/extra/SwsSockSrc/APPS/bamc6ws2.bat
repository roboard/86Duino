@echo off
rem Build all MSVC apps

for /D %%i in (*) do call makevc.bat %%i
pause
