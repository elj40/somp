/*
* Filename:	somp_tester.c
* Date:		27/12/2024 
* Name:		EL Joubert
*
* A program that will run tests on the logic of somps solver (and maybe other
* things) to ensure that we cover edge cases. Hopefully I dont get lazy
*/

#define TODO(msg) printf("[%s, %d] TODO: "msg"\n",__FILE__, __LINE__);
#include <stdio.h>
#include "somp_logic.c"
 
int main()
{
	printf("Linked list tests:\n");
	int list[] = {1,2,3,4,5,6};
	LL_Node * intLL = NULL;
	for (int i = 0; i < 5; i++)
	{
		LL_push(&intLL, list + i);
	}
	LL_print(intLL, printInt);
	LL_remove(&intLL, list+1);
	LL_print(intLL, printInt);
	while (intLL) LL_remove(&intLL, intLL->data);
	LL_print(intLL, printInt);

	LL_free(intLL);
	printf("\n");

	TODO("Test sorting");
	float beamLength = 1.0;
	int pfCount = 0;
	int dfCount = 0;
	int sectionsCount = 0;
	PointForce       pForces[5]   = {0}; 
	DistributedForce dForces[5]   = {0};
	Section          sections[20] = {0};

	beamLength = 1.0;
	dForces[1] = (DistributedForce){ 0, 0.5, {1,0} };
	dForces[0] = (DistributedForce){ 0.25, 0.75, {2,0} };
	dfCount = 2;

	seperateBeamIntoSections(beamLength, pForces, pfCount, dForces, dfCount, sections, &sectionsCount);
}
