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

#define UTILS_IMPLEMENTATION
#include "utils.h"

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"
 
void test(int condition, const char * fail_message);
void testLinkedLists();
void testFloatComparison();
void testSeperateSections();
void testWallReaction();

int main()
{
	testFloatComparison();
	testLinkedLists();
	testSeperateSections();
	testWallReaction();

	printf("\nSolving beam:\n");
	Beam beam = {0};
	beam.length = 1.0;
	beam.sectionsCount = MAX_SECTIONS;

	PointForce pointForces[10]; //TODO: figure out how many we should allocate for
	int pfCount;
	pointForces[0] = (PointForce){ 0.0, 1 };
	pointForces[1] = (PointForce){ 0.25,2 };
	pointForces[2] = (PointForce){ 0.5, 3 };
	pointForces[3] = (PointForce){ 1.0, 4 };
	pfCount = 4;

	DistributedForce distributedForces[10];
	int dfCount;
	distributedForces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	distributedForces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	distributedForces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	distributedForces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;
	solveBeam(&beam, pointForces, pfCount, distributedForces, dfCount);

	printf("Raw:\n");
	printStructArray(beam.raws, beam.sectionsCount, sizeof(Section), printSection);

	printf("Shear:\n");
	printStructArray(beam.shears, beam.sectionsCount, sizeof(Section), printSection);

	printf("Moment:\n");
	printStructArray(beam.moments, beam.sectionsCount, sizeof(Section), printSection);
}
void test(int condition, const char * fail_message)
{
	if (condition) printf(".");
	else printf("F - %s\n", fail_message);
}

void testWallReaction()
{
	printf("Testing wall reaction:\n");
	float wrf;

	Section sections[] = {
		(Section) { .pointForce = 1 },
		(Section) { .pointForce = 2 },
		(Section) { .pointForce = 3 }
	};

	wrf = calculateWallReactionForce(sections, 3);
	test(nearlyEqual(wrf, 6), "Incorrect wall reaction force (sum of point sources)");
	//////////////////////////////////////////////

	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[0] = 1;

	wrf = calculateWallReactionForce(sections, 1);
	test(nearlyEqual(wrf, 1),"Incorrect wall reaction force (integral of y=1)"); 
	/////////////////////////////////////////////
	
	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[1] = 1;

	wrf = calculateWallReactionForce(sections, 1);
	test(nearlyEqual(wrf, 0.5),"Incorrect wall reaction force (integral of y=x)"); 
	//////////////////////////////////////////////
	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[2] = 1;

	wrf = calculateWallReactionForce(sections, 1);
	test(nearlyEqual(wrf, 1.0/3.0),"Incorrect wall reaction force (integral of y=x^2), check MAX_POLYNOMIAL_DEGREE"); 
	//////////////////////////////////////////////
	sections[0] = (Section){ .start = 0, .end = 3 };
	sections[1] = (Section){ .start = 0, .end = 2 };
	sections[2] = (Section){ .start = 0, .end = 1 };

	sections[0].polynomial[0] = 1;
	sections[1].polynomial[1] = 1;
	sections[2].polynomial[2] = 1;

	wrf = calculateWallReactionForce(sections, 3);
	test(nearlyEqual(wrf, (1.0/3.0) + 2.0 + 3.0),"Incorrect wall reaction force (sum of polys), check MAX_POLYNOMIAL_DEGREE"); 
	printf("\n");
}
void testSeperateSections()
{
	printf("Section tests:\n");
	TODO("Write more section tests");
	int r = 1;

	float beamLength = 1.0;
	int pfCount = 0;
	int dfCount = 0;
	int sectionsCount = MAX_SECTIONS;
	PointForce       pForces[5]   = {0}; 
	DistributedForce dForces[5]   = {0};
	Section          sections[20] = {0};

	beamLength = 1.0;
	pForces[0] = (PointForce){ 0.0, 1 };
	pForces[1] = (PointForce){ 0.25,2 };
	pForces[2] = (PointForce){ 0.5, 3 };
	pForces[3] = (PointForce){ 1.0, 4 };
	pfCount = 4;
	dForces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	dForces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	dForces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	dForces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;

	seperateBeamIntoSections(beamLength, pForces, pfCount, dForces, dfCount, sections, &sectionsCount);
	/* printStructArray(sections, sectionsCount, sizeof(Section), printSection); */

	Section expectedSections[] = {
		(Section){ .start = 0.000000, .end = 0.250000, .pointForce =  1.000000 },
		(Section){ .start = 0.250000, .end = 0.500000, .pointForce =  2.000000 },
		(Section){ .start = 0.500000, .end = 0.650000, .pointForce =  3.000000 },
		(Section){ .start = 0.650000, .end = 0.750000, .pointForce =  0.000000 },
		(Section){ .start = 0.750000, .end = 0.950000, .pointForce =  0.000000 },
		(Section){ .start = 0.950000, .end = 1.000000, .pointForce =  0.000000 },
	};

	if (sectionsCount == ArrayCount(expectedSections)) printf(".");
	else printf("F - wrong number of sections\n");

	for (int i = 0; i < sectionsCount; i++)
	{
		if (!compSections(sections[i], expectedSections[i]))
		{
			printf("F - Section does not match expected section:\n");
			printf("Found:    "); printSection(&sections[i]);
			printf("Expected: "); printSection(&expectedSections[i]);
			r = 0;
		} 
	}
	if (r) printf(".");

	printf("\n");
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
