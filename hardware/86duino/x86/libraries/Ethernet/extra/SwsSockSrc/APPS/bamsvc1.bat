@echo off
rem Build all MSVC1 DOS apps

for /D %%i in (*) do call makevc1.bat %%i
pause
