#!/bin/bash
set -xe

# gcc -ggdb -o "somp.exe" somp.c somp_logic.c utils.c -L%SDL_PATH_LIB% -I%SDL_PATH_INCLUDE% -lmingw32 -lSDL2main -lSDL2 
gcc -Wall -Wextra -ggdb -o tester somp_tester.c -lm

# somp.exe
./tester

