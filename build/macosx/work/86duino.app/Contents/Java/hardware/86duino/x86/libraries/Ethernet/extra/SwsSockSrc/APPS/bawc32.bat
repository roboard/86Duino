@echo off
rem Build all WatcomC DOS32 apps

for /D %%i in (*) do call makewc32.bat %%i
echo.
pause
