#define ELNOB_IMPLEMENTATION
#include "elnob.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

bool build_somp()
{
    // gcc -ggdb -o "somp.exe" somp.c somp_logic.c utils.c -L%SDL_PATH_LIB% -I%SDL_PATH_INCLUDE% -lmingw32 -lSDL2main -lSDL2 
    // somp.exe
#define SDL_PATH_INCLUDE "/home/eli/Software/thirdparty/SDL-release-2.30.12/include/" 
    printf("-I"SDL_PATH_INCLUDE"\n");
    const char * compile[] = { "gcc", "-ggdb", "-o", "somp", "somp.c", "somp_logic.c", "utils.c", "-I"SDL_PATH_INCLUDE, "-lSDL2main", "-lSDL2", NULL }; 
    if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return false;
    const char * run[] = { "./somp", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(run), run)) return false;

    return true;
}

bool build_tests()
{
    const char * compile[] = { "gcc","-Wall","-Wextra","-ggdb","-o","tester","somp_tester.c","-lm", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return false;
    const char * run[] = { "./tester", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(run), run)) return false;

    return true;
}
int main(int argc, const char * argv[])
{

    struct stat code_stat;
    struct stat libh_stat;
    struct stat exec_stat;
	stat("elnob.c", &code_stat);
	stat("elnob.h", &libh_stat);
	stat("elnob.out", &exec_stat);

    if (code_stat.st_mtime > exec_stat.st_mtime
        || libh_stat.st_mtime > exec_stat.st_mtime)
    {
        printf("=========== ELNOB RECOMPILE =============\n");
        const char * compile[] = {"gcc", "-o", "elnob.out", "elnob.c", NULL};
        if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return 2;

        const char * rerun[] = {"./elnob.out",NULL};
        if (!run_command_sync(ELNOB_ARRAY_SIZE(rerun), rerun)) return 2;
        return 0;
    }

    printf("============= ELNOB RUN =================\n");

    if (argc > 1 && strcmp(argv[1], "test") == 0)
    {
        if (!build_tests()) return 1;
    } else 
    {
        if (!build_somp()) return 1;
    }


    return 0;
}
