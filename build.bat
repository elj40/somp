@echo off
set SDL_PATH=C:\Users\elaij\Software\SDL2\i686-w64-mingw32

gcc -ggdb -o "somp.exe" somp.c somp_logic.c utils.c -L%SDL_PATH%\lib -I%SDL_PATH%\include\SDL2 -lmingw32 -lSDL2main -lSDL2
REM gcc -Wall -Wextra -ggdb -o tester.exe somp_tester.c utils.c somp_logic.c 

if %errorlevel% neq 0 exit /b %errorlevel%

somp.exe
REM tester.exe
