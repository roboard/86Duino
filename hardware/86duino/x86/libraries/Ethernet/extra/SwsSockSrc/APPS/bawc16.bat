@echo off
rem Build all WatcomC DOS16 apps

for /D %%i in (*) do call makewc16.bat %%i
echo.
pause
