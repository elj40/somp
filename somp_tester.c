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
#include <stdlib.h>
#include <assert.h>

#define UTILS_IMPLEMENTATION
#include "utils.h"

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

#include "ejtest.h"
 
void test(int condition, const char * fail_message);
void testLinkedLists();
void testFloatComparison();
void testSeperateSections();
void testWallReaction();
void testSolveBeam();
void testDynamicArrayAppend();

typedef struct {
    int * items;
    int capacity;
    int count;
} Ints;

void printInts(Ints ints)
{
    printf("Ints: [ ");
    for (int i = 0; i < ints.count; i++)
    {
        printf("%d", ints.items[i]);
        if (i < ints.count - 1) printf(", ");
    }
    printf(" ] %d elements, %d capacity\n", ints.count, ints.capacity);
};

int main()
{
	testFloatComparison();
	testLinkedLists();
	testSeperateSections();
	testWallReaction();
    testSolveBeam();


    testDynamicArrayAppend();
    return 0;
}
void test(int condition, const char * fail_message)
{
	if (condition) printf(".");
	else printf("F - %s\n", fail_message);
}

void testDynamicArrayAppend()
{
    bool R = true;
    Ints ints = {0};
    for (int i = 0; i < 100; i++)
    {
        DynamicArrayAppend(&ints, i);
        ejtest_expect_bool(&R, ints.count <= ints.capacity, true);
        ejtest_expect_int( &R, ints.items[i], i);
    }

    free(ints.items);
    ejtest_print_result("testDynamicArrayAppend", R);
}

void testSolveBeam()
{
    bool R = true;
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

    Section expected_raw[] = {
        { .start = 0.000, .end = 0.250, .pointForce = 1.000, .polynomial = {1.00, 0.00, 0.00, 0.00}},
        { .start = 0.250, .end = 0.500, .pointForce = 2.000, .polynomial = {3.00, 0.00, 0.00, 0.00}},
        { .start = 0.500, .end = 0.650, .pointForce = 3.000, .polynomial = {2.00, 0.00, 0.00, 0.00}},
        { .start = 0.650, .end = 0.750, .pointForce = 0.000, .polynomial = {6.00, 0.00, 0.00, 0.00}},
        { .start = 0.750, .end = 0.950, .pointForce = 0.000, .polynomial = {7.00, 0.00, 0.00, 0.00}},
        { .start = 0.950, .end = 1.000, .pointForce = 0.000, .polynomial = {3.00, 0.00, 0.00, 0.00}},
    };
    Section expected_shear[] = {
        { .start = 0.000, .end = 0.250, .pointForce = 0.000, .polynomial = {8.45, -1.00, -0.00, -0.00}},
        { .start = 0.250, .end = 0.500, .pointForce = 0.000, .polynomial = {6.95, -3.00, -0.00, -0.00}},
        { .start = 0.500, .end = 0.650, .pointForce = 0.000, .polynomial = {3.45, -2.00, -0.00, -0.00}},
        { .start = 0.650, .end = 0.750, .pointForce = 0.000, .polynomial = {6.05, -6.00, -0.00, -0.00}},
        { .start = 0.750, .end = 0.950, .pointForce = 0.000, .polynomial = {6.80, -7.00, -0.00, -0.00}},
        { .start = 0.950, .end = 1.000, .pointForce = 0.000, .polynomial = {3.00, -3.00, -0.00, -0.00}},
    };
    Section expected_moment[] = {
        { .start = 0.000, .end = 0.250, .pointForce = 0.000, .polynomial = {-4.24, 8.45, -0.50, -0.00}},
        { .start = 0.250, .end = 0.500, .pointForce = 0.000, .polynomial = {-3.80, 6.95, -1.50, -0.00}},
        { .start = 0.500, .end = 0.650, .pointForce = 0.000, .polynomial = {-2.18, 3.45, -1.00, -0.00}},
        { .start = 0.650, .end = 0.750, .pointForce = 0.000, .polynomial = {-3.02, 6.05, -3.00, -0.00}},
        { .start = 0.750, .end = 0.950, .pointForce = 0.000, .polynomial = {-3.30, 6.80, -3.50, -0.00}},
        { .start = 0.950, .end = 1.000, .pointForce = 0.000, .polynomial = {-1.50, 3.00, -1.50, -0.00}},
    };

    ejtest_expect_int(&R, ArrayCount(expected_raw), beam.sectionsCount);

    for (int i = 0; i < beam.sectionsCount; i++)
    {
        ejtest_expect_struct(&R, beam.raws[i], expected_raw[i], comp_sections);
        ejtest_expect_struct(&R, beam.shears[i], expected_shear[i], comp_sections);
        ejtest_expect_struct(&R, beam.moments[i], expected_moment[i], comp_sections);
    };

    ejtest_print_result("testSolveBeam", R);
}

void testWallReaction()
{
    bool passed = true;
	float wrf;

	Section sections[] = {
		(Section) { .pointForce = 1 },
		(Section) { .pointForce = 2 },
		(Section) { .pointForce = 3 }
	};

	wrf = calculateWallReactionForce(sections, 3);
    ejtest_expect_float(&passed, wrf, 6.0);
	//////////////////////////////////////////////

	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[0] = 1;

	wrf = calculateWallReactionForce(sections, 1);
    ejtest_expect_float(&passed, wrf, 1.0);
	/////////////////////////////////////////////
	
	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[1] = 1;

	wrf = calculateWallReactionForce(sections, 1);
    ejtest_expect_float(&passed, wrf, 0.5);
	//////////////////////////////////////////////
	sections[0] = (Section){ .start = 0, .end = 1 };
	sections[0].polynomial[2] = 1;

	wrf = calculateWallReactionForce(sections, 1);
    ejtest_expect_float(&passed, wrf, (1.0/3.0));
	//////////////////////////////////////////////
	sections[0] = (Section){ .start = 0, .end = 3 };
	sections[1] = (Section){ .start = 0, .end = 2 };
	sections[2] = (Section){ .start = 0, .end = 1 };

	sections[0].polynomial[0] = 1;
	sections[1].polynomial[1] = 1;
	sections[2].polynomial[2] = 1;

	wrf = calculateWallReactionForce(sections, 3);
    ejtest_expect_float(&passed, wrf, (1.0/3.0) + 2.0 + 3.0);
	ejtest_print_result("testWallReaction", passed);
}
void testSeperateSections()
{
	TODO("Write more section tests");
    bool R = true;

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

	Section expectedSections[] = {
		(Section){ .start = 0.000000, .end = 0.250000, .pointForce =  1.000000 },
		(Section){ .start = 0.250000, .end = 0.500000, .pointForce =  2.000000 },
		(Section){ .start = 0.500000, .end = 0.650000, .pointForce =  3.000000 },
		(Section){ .start = 0.650000, .end = 0.750000, .pointForce =  0.000000 },
		(Section){ .start = 0.750000, .end = 0.950000, .pointForce =  0.000000 },
		(Section){ .start = 0.950000, .end = 1.000000, .pointForce =  0.000000 },
	};

    ejtest_expect_int(&R, ArrayCount(expectedSections), sectionsCount);

	for (int i = 0; i < sectionsCount; i++)
	{
        ejtest_expect_struct(&R, sections[i], expectedSections[i], comp_sections);
		if (!comp_sections(&sections[i], &expectedSections[i]))
		{
			printf("Found:    "); printSection(&sections[i]);
			printf("Expected: "); printSection(&expectedSections[i]);
		} 
	}

    ejtest_print_result("testSeperateSections", R);
}
void testFloatComparison()
{
    bool R = true;

	float a, b; 

	a = 0.0;
	b = 0.0;
    ejtest_expect_bool(&R, nearly_equal(a, b), true);

	a = 0.25;
	b = 0.0;
    ejtest_expect_bool(&R, nearly_equal(a, b), false);

    ejtest_print_result("testFloatComparison", R);
}
void testLinkedLists()
{
    bool R = true;

	LL_Node * test;
	int list[] = {1,2,3,4,5,6};
	LL_Node * intLL = NULL;

	for (int i = 0; i < 5; i++)
	{
		LL_push(&intLL, list + i);
	}

	//Succesfully pushed
	test = intLL;
	for (int i = 0; i < 5; i++)
	{
        ejtest_expect_int(&R, *((int *)(test->data)), list[i]);
		test = test->next;
	}

	// Remove head
	LL_remove(&intLL, list);
    ejtest_expect_int(&R, *((int *)(intLL->data)), list[1]);

	// Remove 2nd
	LL_remove(&intLL, list+2);
	ejtest_expect_int(&R, *((int*)intLL->next->data), list[3]); 

	//Remove all
	while (intLL) LL_remove(&intLL, intLL->data);
	ejtest_expect_bool(&R, intLL, false); 

	LL_free(intLL);
    ejtest_print_result("testLinkedLists", R);
}
