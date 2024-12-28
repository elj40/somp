@echo off
gcc -ggdb -o "somp.exe" main.c 
gcc -ggdb -o tester.exe somp_tester.c

if %errorlevel% neq 0 exit /b %errorlevel%

REM somp.exe
tester.exe
