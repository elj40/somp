#ifndef _ELNOB_H_
#define _ELNOB_H_

#define ELNOB_VERSION 0.1.0

typedef struct {
    char ** items;
    int count;
    int capacity;
} Command;

int run_command_sync(int argc, const char * argv[]);
int elnob_rebuild_elnob(int argc, const char * argv[]);
void elnob_print_command(Command cmd);

#ifndef ELNOB_QUIET
    #define elnob_recompile_msg() printf("========================= ELNOB RECOMPILE ============================\n");
    #define elnob_run_msg()       printf("========================= ELNOB RUN ==================================\n");
#else
    #define elnob_recompile_msg()
    #define elnob_run_msg()
#endif
#define ELNOB_ARRAY_SIZE(a) sizeof((a))/sizeof((a)[0])

#ifdef ELNOB_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <assert.h>

#define ELNOB_DEFAULT_DA_CAPACITY 5
#define elnob_da_append(da, item) \
    do { \
        if ((da)->count >= (da)->capacity) \
        { \
            if ((da)->capacity == 0) (da)->capacity = ELNOB_DEFAULT_DA_CAPACITY; \
            else (da)->capacity *= 2; \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof((da)->items[0])); \
            assert((da)->items != NULL); \
        } \
        (da)->items[(da)->count] = item; \
        (da)->count++; \
    } while(0);

#define elnob_cmd_append(cmd, item) elnob_da_append((cmd), (item))
#define elnob_cmd_append_many(cmd, ...) elnob_cmd_append_many_func((cmd), __VA_ARGS__, NULL)
void elnob_cmd_append_many_func(Command * cmd, ...)
{
    char * item = NULL;
    va_list vargs;
    va_start(vargs, cmd);
    while ((item = va_arg(vargs, char *)))
    {
        elnob_cmd_append(cmd, item);
    };
};

int elnob_rebuild_elnob(int argc, const char * argv[])
{
    if (argc <= 0) return 0;
    //TODO: make all the rebuild yourself into macr or function
    struct stat code_stat;
    struct stat libh_stat;
    struct stat exec_stat;

    const char * exec_name = argv[0];
    // TODO: make this no longer a hard coded value
	stat("elnob.c", &code_stat);
	stat(__FILE__, &libh_stat);
	stat(exec_name, &exec_stat);


    //printf("code: %d\nlibh: %d\nexec: %d\n",
     //       code_stat.st_mtime, libh_stat.st_mtime, exec_stat.st_mtime);

    if (code_stat.st_mtime > exec_stat.st_mtime
        || libh_stat.st_mtime > exec_stat.st_mtime)
    {
        elnob_recompile_msg();
        const char * compile[] = {"cc", "-g", "-o", exec_name, "elnob.c", NULL};
        if (!run_command_sync(ELNOB_ARRAY_SIZE(compile), compile)) return 0;

        const char * rerun[] = { exec_name ,NULL};
        if (!run_command_sync(ELNOB_ARRAY_SIZE(rerun), rerun)) return 0;
        exit(0);
    }
    return 1;
}

// argc includes the NULL at the end
int elnob_run_command_sync(Command cmd)
{
	if (cmd.count <= 0) {
		printf("Command must have at least one argument\n");
		return 0;
	}
	pid_t pid = fork();

	if (pid == 0)
	{
        printf("++ "); elnob_print_command(cmd);
		int r = execvp(cmd.items[0], (char * const *) cmd.items);
		if (r < 0) {
            printf("Failure:" ); elnob_print_command(cmd);
            exit(69);
        }
        exit(EXIT_SUCCESS);
	}

    int exit_status = 0;
	wait(&exit_status);

    //printf("exit_status: %d %d\n", exit_status, WEXITSTATUS(exit_status));
    if (WEXITSTATUS(exit_status) != 0)
    {
        printf("ERROR: failed to run command:"); elnob_print_command(cmd);
        return 0;
    }
    return 1;
}

void elnob_print_command(Command cmd)
{
    for (int i = 0; i < cmd.count; i++)
    {
        if (i > 0) printf(" ");
        printf("%s", cmd.items[i]);
    }
    printf("\n");
};
int run_command_sync(int argc, const char * argv[])
{
	if (argc <= 0) {
		printf("Command must have at least one argument\n");
		return 0;
	}
	if (argv[argc-1] != NULL) {
		printf("Command %s must be terminated by NULL\n", argv[0]);
		return 0;
	}
	pid_t pid = fork();

	if (pid == 0)
	{
		printf("Running command: %s...\n", argv[0]);
		int r = execvp(argv[0], (char * const *) argv);
		if (r < 0) {
            printf("Failed to run command: %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
	}

    int exit_status = 0;
	wait(&exit_status);

    //printf("exit_status: %d %d\n", exit_status, WEXITSTATUS(exit_status));
    if (WEXITSTATUS(exit_status) != 0)
    {
        printf("ERROR: failed to run command: %s, exiting\n", argv[0]);
        return 0;
    }
    return 1;
}
#endif // ELNOB_IMPLEMENTATION
#endif // _ELNOB_H_


// NOTE!!! Examples are made as elnob is developed, 
// so some may stop working at some point

// =================== EXAMPLE 1 ====================
// Pure C, no abstraction
//
// int main()
//{
//	printf("Hello Elnob\n");
//	pid_t pid = fork();
//
//	if (pid == 0)
//	{
//		int result = execlp("gcc", "gcc", "-Wall", "-Wextra", "-o",
//		"elnob", "elnob.c", NULL);
//		if (result < 0) { 
//			printf("Failed to run command\n"); 
//			return 1; 
//		}
//		return 0;
////	} else
//	{
//		wait(NULL);
//		printf("I want this to run as well\n");
//	}
//	return 0;
//}

// =================== EXAMPLE 2 ====================
// Using run_command
/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#define ELNOB_IMPLEMENTATION
#include "elnob.h"

// NOTE, we get away with wait(NULL) because we make sure that we do not fork
// more than once, a more secure function would be better
int main()
{
	struct stat code_stat;
	struct stat exec_stat;
	stat("elnob.c", &code_stat);
	stat("elnob.out", &exec_stat);

	pid_t pid;

	if (code_stat.st_mtime > exec_stat.st_mtime)
	{
		printf("elnob.c edited, need to recompile\n");
		const char * compile[] = {"gcc", "-o", "elnob.out", "elnob.c", NULL};
		run_command(4, compile);

		const char * rerun[] = {"./elnob.out", NULL};
		run_command(1,  rerun);
		return 0;
	}
	const char * cmd[] = {"ls", "-a", NULL};
	run_command(2, cmd);
	return 0;
}
*/
