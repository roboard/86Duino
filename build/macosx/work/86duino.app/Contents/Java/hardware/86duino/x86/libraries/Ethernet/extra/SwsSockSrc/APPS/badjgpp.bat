@echo off
rem Build all DJGPP apps

for /D %%i in (*) do call makedjg.bat %%i
echo.
pause
