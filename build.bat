@echo off

if not exist .\SDL2.dll (
	echo "SDL2.dll not found"
	exit
)
set SDL_INCLUDE=C:\Users\elaij\Software\SDL2\i686-w64-mingw32\include\
set SDL_LIBS=C:\Users\elaij\Software\SDL2\i686-w64-mingw32\lib\
set SDL_TTF_LIB= C:\Users\elaij\Software\SDL2\SDL2_ttf-devel-2.22.0-mingw\SDL2_ttf-2.22.0\i686-w64-mingw32\lib\

gcc -g -Wall -o "somp.exe" main.c -I%SDL_INCLUDE% -L%SDL_LIBS% -L%SDL_TTF_LIB% -lmingw32 -lSDL2_ttf -lSDL2main -lSDL2

if %errorlevel% neq 0 exit /b %errorlevel%

somp.exe
