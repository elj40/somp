@echo off
REM On adams laptop
REM set SDL_PATH=C:\Users\elaij\Software\SDL2\i686-w64-mingw32 

REM Barga setup
set SDL_PATH_INCLUDE=C:\Users\28178564.STB\Downloads\SDL2-devel-2.32.0-mingw\SDL2-2.32.0\i686-w64-mingw32\include\SDL2
set SDL_PATH_LIB=C:\Users\28178564.STB\Downloads\SDL2-devel-2.32.0-mingw\SDL2-2.32.0\i686-w64-mingw32\lib


gcc -ggdb -o "somp.exe" somp.c somp_logic.c utils.c -L%SDL_PATH_LIB% -I%SDL_PATH_INCLUDE% -lmingw32 -lSDL2main -lSDL2 
REM gcc -Wall -Wextra -ggdb -o tester.exe somp_tester.c utils.c somp_logic.c 

if %errorlevel% neq 0 exit /b %errorlevel%

somp.exe
REM tester.exe
