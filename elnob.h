#ifndef _ELNOB_H_
#define _ELNOB_H_

// So far there are no functions but here is a short example

// =================== EXAMPLE ====================
// int main()
//{
//	printf("Hello Elnob\n");
//	pid_t pid = fork();
//
//	if (pid == 0)
//	{
//		int result = execlp("gcc", "gcc", "-Wall", "-Wextra", "-o", "elnob", "elnob.c", NULL);
//		if (result < 0) { printf("Failed to run command\n"); return 1; }
//		return 0;
////	} else
//	{
//		wait(NULL);2
//		printf("I want this to run as well\n");
//	}
//
//	return 0;
//}
// =================================================

#ifdef ELNOB_IMPLEMENTATION
#include <unistd.h>
#include <sys/wait.h> 
#endif // ELNOB_IMPLEMENTATION

#endif // _ELNOB_H_
