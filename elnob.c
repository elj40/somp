#define ELNOB_IMPLEMENTATION
#include "elnob/elnob.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

//TODO: update this to latest version

bool build_cli()
{
    const char * output = "./somp.out";
    const char * compile[] = { "gcc", "-ggdb", "-o", output, "somp_cli.c","-lm", NULL }; 
    if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return false;
    const char * run[] = { output, NULL };
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

bool build_gui(Command cmd)
{
    cmd.count = 0;
#define SOMP_HOTRELOAD
#ifndef SOMP_HOTRELOAD
    elnob_cmd_append_many(&cmd, "gcc","-Wall","-Wextra","-ggdb");
    elnob_cmd_append_many(&cmd, "-o","somp_gui.out","somp_gui.c");
    elnob_cmd_append_many(&cmd, "-L./SDL/build/", "-I./SDL/include");
    elnob_cmd_append_many(&cmd, "-L./SDL3_ttf-3.2.2/build/", "-I./SDL3_ttf-3.2.2/include/");
    elnob_cmd_append_many(&cmd, "-Wl,-rpath,./SDL/build,./SDL3_ttf-3.2.2/build/");
    elnob_cmd_append_many(&cmd, "-lSDL3", "-lm");
    if (!elnob_run_command_sync(cmd)) return false;
#else
    elnob_cmd_append_many(&cmd, "gcc", "-fPIC", "-shared", "-Wall","-Wextra","-ggdb");
    elnob_cmd_append_many(&cmd, "-o","somp_gui.so","somp_gui.c");
    elnob_cmd_append_many(&cmd, "-L./SDL/build/", "-I./SDL/include");
    elnob_cmd_append_many(&cmd, "-L./SDL3_ttf-3.2.2/build/", "-I./SDL3_ttf-3.2.2/include/");
    elnob_cmd_append_many(&cmd, "-Wl,-rpath,./SDL/build:./SDL3_ttf-3.2.2/build/");
    elnob_cmd_append_many(&cmd, "-lSDL3", "-lSDL3_ttf", "-lm");
    elnob_cmd_append_many(&cmd, "-DSDL_MAIN_HANDLED");
    if (!elnob_run_command_sync(cmd)) return false;

    cmd.count = 0;
    elnob_cmd_append_many(&cmd, "gcc", "-Wall","-Wextra","-ggdb");
    elnob_cmd_append_many(&cmd, "-o","somp.out","somp_hot.c");
    elnob_cmd_append_many(&cmd, "-Wl,-rpath,.", "-ldl");
    if (!elnob_run_command_sync(cmd)) return false;
#endif

    return true;
}

bool build_gui2(Command cmd)
{
    cmd.count = 0;
    elnob_cmd_append_many(&cmd, "gcc", "-fPIC", "-shared", "-Wall","-Wextra","-ggdb");
    elnob_cmd_append_many(&cmd, "-o","somp_gui.so","somp_gui2.c");
    elnob_cmd_append_many(&cmd, "-L./SDL/build/", "-I./SDL/include");
    elnob_cmd_append_many(&cmd, "-L./SDL3_ttf-3.2.2/build/", "-I./SDL3_ttf-3.2.2/include/");
    elnob_cmd_append_many(&cmd, "-Wl,-rpath,./SDL/build:./SDL3_ttf-3.2.2/build/");
    elnob_cmd_append_many(&cmd, "-lSDL3", "-lSDL3_ttf", "-lm");
    elnob_cmd_append_many(&cmd, "-DSDL_MAIN_HANDLED");
    if (!elnob_run_command_sync(cmd)) return false;

    cmd.count = 0;
    elnob_cmd_append_many(&cmd, "gcc", "-Wall","-Wextra","-ggdb");
    elnob_cmd_append_many(&cmd, "-o","somp.out","somp_hot.c");
    elnob_cmd_append_many(&cmd, "-Wl,-rpath,.", "-ldl");
    if (!elnob_run_command_sync(cmd)) return false;

    return true;
}

int main(int argc, const char * argv[])
{
    elnob_rebuild_elnob(argc, argv);

    elnob_run_msg();
    Command cmd = {0};

    if (argc > 1)
    {
        if(strcmp(argv[1], "test") == 0 && !build_tests()) return 1;
        else if(strcmp(argv[1], "gui") == 0 && !build_gui(cmd)) return 1;
        else if(strcmp(argv[1], "cli") == 0 && !build_cli()) return 1;
    } else
    {
        if (!build_gui2(cmd)) return 1;
    }
    return 0;
}
