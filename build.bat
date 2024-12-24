@echo off
gcc -ggdb -o "somp.exe" main.c 

if %errorlevel% neq 0 exit /b %errorlevel%

somp.exe
