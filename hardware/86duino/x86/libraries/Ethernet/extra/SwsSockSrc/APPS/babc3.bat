@echo off
rem Build all BorlandC DOS apps

for /D %%i in (*) do call makebc3.bat %%i
pause
