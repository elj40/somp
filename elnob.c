#define ELNOB_IMPLEMENTATION
#include "elnob.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

bool build_somp_cli()
{
    const char * compile[] = { "gcc", "-ggdb", "-o", "somp.out", "somp_cli.c","-lm", NULL }; 
    if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return false;
    const char * run[] = { "./somp.out", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(run), run)) return false;
    return true;
}

bool build_tests()
{
    const char * compile[] = { "gcc","-Wall","-Wextra","-ggdb","-o","tester.out","somp_tester.c","-lm", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return false;
    const char * run[] = { "./tester.out", NULL };
    if (!run_command_sync(ELNOB_ARRAY_SIZE(run), run)) return false;

    return true;
}
int main(int argc, const char * argv[])
{
    //TODO: make all the rebuild yourself into macr or function
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
        if (!build_somp_cli()) return 1;
    }


    return 0;
}
