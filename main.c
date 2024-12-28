/*
* Filename:	main.c
* Date:		24/12/2024 
* Name:		EL Joubert
*
* SOMP
* Made to simulate the stress in a beam caused by simple forces and display the
* functions that these forces produce
*/

#define TODO(msg) printf("[%s, %d] TODO: "msg"\n",__FILE__, __LINE__);

#include <stdio.h>
#include "somp_logic.c"

int main()
{
	printf("Hello SOMP\n");
	solveBeam();
	return 0;
}
