@echo off
set SDL_PATH=C:\Users\elaij\Software\SDL2\i686-w64-mingw32

REM gcc -ggdb -o "somp.exe" main.c  -L%SDL_PATH%\lib -I%SDL_PATH%\include\SDL2 -lmingw32 -lSDL2main -lSDL2
gcc -ggdb -o tester.exe somp_tester.c utils.c somp_logic.c 

if %errorlevel% neq 0 exit /b %errorlevel%

REM somp.exe
tester.exe
