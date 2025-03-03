#define ELNOB_IMPLEMENTATION
#include "elnob.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char * argv[])
{
	printf("Hello somp!\n");	
	// This will be sync compile

	// Compile
	pid_t pid = fork();
	if (pid == 0)
	{
		int r = execlp("gcc", "gcc", "-Wall", "-Wextra", "-ggdb", "-o", "tester", "somp_tester.c", "-lm", NULL);
		if (r < 0) return 1;
		return 0;
	} 

	wait(NULL);
	printf("This should run after compilation\n");

	pid = fork();
	if (pid == 0)
	{
		int r = execlp("./tester", "./tester");
		if (r < 0) return 1;
		return 0;
	}

	return 0;
}
