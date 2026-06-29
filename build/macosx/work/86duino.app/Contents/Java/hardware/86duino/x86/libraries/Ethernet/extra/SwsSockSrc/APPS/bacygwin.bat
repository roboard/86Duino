@echo off
rem Build all CYGWIN apps

for /D %%i in (*) do call makecygw.bat %%i
echo.
pause
