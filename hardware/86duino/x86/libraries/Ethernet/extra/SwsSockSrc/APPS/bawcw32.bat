@echo off
rem Build all WatcomC Win32 apps

for /D %%i in (*) do call makwcw32.bat %%i
echo.
pause
