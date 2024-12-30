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
 
void testLinkedLists();
void testFloatComparison();

int main()
{
	testFloatComparison();
	testLinkedLists();
	float beamLength = 1.0;
	int pfCount = 0;
	int dfCount = 0;
	int sectionsCount = 0;
	PointForce       pForces[5]   = {0}; 
	DistributedForce dForces[5]   = {0};
	Section          sections[20] = {0};

	beamLength = 1.0;
	pForces[0] = (PointForce){ 0.0, 1 };
	pForces[1] = (PointForce){ 0.25, 2 };
	pForces[2] = (PointForce){ 0.5, 3 };
	pfCount = 3;
	dForces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	dForces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	dForces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	dForces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;

	seperateBeamIntoSections(beamLength, pForces, pfCount, dForces, dfCount, sections, &sectionsCount);

	printStructArray(sections, sectionsCount, sizeof(Section), printSection);
}

void testFloatComparison()
{
	printf("Floating point comparison tests\n");
	float a, b; 
	int r;

	a = 0.0;
	b = 0.0;
	r = nearlyEqual(a, b);
	if (r) printf(".");
	else printf("F (%d): 0 ~= 0\n", r);

	a = 0.25;
	b = 0.0;
	r = nearlyEqual(a, b);
	if (!r) printf(".");
	else printf("F (%d): 0.25 != 0.0\n", r);

	printf("\n");
}
void testLinkedLists()
{
	LL_Node * test;
	int result;
	printf("Linked list tests:\n");
	int list[] = {1,2,3,4,5,6};
	LL_Node * intLL = NULL;
	for (int i = 0; i < 5; i++)
	{
		LL_push(&intLL, list + i);
	}
	//Succesfully pushed
	test = intLL;
	result = 1;
	for (int i = 0; i < 5; i++)
	{
		if (*((int *)(test->data)) != list[i]) result=0;
		test = test->next;
	}
	if (result) printf(".");
	else printf("\nF: Push failed\n");

	// Remove head
	result = 1;
	LL_remove(&intLL, list);
	if (*((int*)intLL->data) != list[1]) result = 0; 
	if (result) printf(".");
	else printf("\nF: Remove head failed\n");

	// Remove 2nd
	result = 1;
	LL_remove(&intLL, list+2);
	if (*((int*)intLL->next->data) != list[3]) result = 0; 
	if (result) printf(".");
	else printf("\nF: Remove 2nd failed\n");

	//Remove all
	result = 1;
	while (intLL) LL_remove(&intLL, intLL->data);
	if (intLL) result = 0;
	if (result) printf(".");
	else printf("\nF: Remove all failed\n");

	printf("\n");
	LL_free(intLL);
}
