#ifndef _ELNOB_H_
#define _ELNOB_H_

#define ELNOB_ARRAY_SIZE(a) sizeof((a))/sizeof((a)[0])
int run_command_sync(int argc, const char * argv[]);

#ifdef ELNOB_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 

// argc includes the NULL at the end
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
		printf("Running custom command: %s\n", argv[0]);
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
