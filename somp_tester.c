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

#define SOMP_IO_IMPLEMENTATION
#include "somp_io.h"

#include "ejtest/ejtest.h"
 
void testLinkedLists();
void testFloatComparison();

void testSeperateSections();

void testWallReactionForce();
void testWallReactionMoment();

void testCompBeams();
void testReadInput();
void testReadBeamInput();
void testReadPointforceInput();
void testReadDistribforceInput();

void testExample_Empty();
void testExample_A();
void testExample_B();
void testExample_C();
void testExample_6_2();
void testExample_6_7();

void testDoubleSameSolve();
void testDoubleDiffSolve();

int main()
{
	testFloatComparison();
	testLinkedLists();
	testSeperateSections();

	testWallReactionForce();
	testWallReactionMoment();
    testCompBeams();

    testReadBeamInput();
    testReadPointforceInput();
    testReadDistribforceInput();
    testReadInput();

    testExample_Empty();
    testExample_A();
    testExample_B();
    testExample_C();
    testExample_6_2();
    testExample_6_7();

    testDoubleSameSolve();
    testDoubleDiffSolve();
    return 0;
}
#define TEST_BEGIN(name) void name() {\
    bool R = true;\
    const char * test_name = #name;
#define TEST_END() ejtest_print_result(test_name, R); }
TEST_BEGIN(testDoubleDiffSolve)
{
    Beam beam = {0};
    beam.length = 1.0;
    beam.sections_count = MAX_SECTIONS;

    PointForces point_forces = {0};
    DistributedForces distributed_forces = {0};

    DynamicArrayAppend(&distributed_forces, ((DistributedForce){ .start = 0.5, .end = 1, .polynomial = {2,0}}));

    bool solved = solveBeam(
            &beam,
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );

    ejtest_expect_bool(&R, solved, true);

    ejtest_expect_int(&R, beam.sections_count, 3);
    ejtest_expect_float(&R, beam.wall_reaction_force, 1);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -1*0.75);

    DynamicArrayAppend(&point_forces, ((PointForce){ .distance = 0.25, .force = 1}));
    solved = solveBeam(
            &beam,
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );


    ejtest_expect_bool(&R, solved, true);

    ejtest_expect_int(&R, beam.sections_count, 4);
    ejtest_expect_float(&R, beam.wall_reaction_force, 2);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -1*0.75-1*0.25);
} TEST_END();
#define TEST_DIR_NAME "./tests"

TEST_BEGIN(testDoubleSameSolve)
{
    Beam beam = {0};
    beam.length = 1.0;
    beam.sections_count = MAX_SECTIONS;

    PointForces point_forces = {0};
    DistributedForces distributed_forces = {0};

    DynamicArrayAppend(&distributed_forces, ((DistributedForce){ .start = 0.5, .end = 1, .polynomial = {2,0}}));

    bool solved = solveBeam(
            &beam,
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );
    solved = solveBeam(
            &beam,
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );
    ejtest_expect_bool(&R, solved, true);

    ejtest_expect_int(&R, beam.sections_count, 3);
    ejtest_expect_float(&R, beam.wall_reaction_force, 1);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -1*0.75);
} TEST_END();
#define TEST_DIR_NAME "./tests"
void testExample_6_7()
{
    bool R = true;

    FILE * file = fopen(TEST_DIR_NAME"/example_6_7.txt", "r");
    assert(file != NULL);

    Beam beam = {0};
    PointForces point_forces = {0};
    DistributedForces distrib_forces = {0};
    
    ejtest_expect_bool(&R, read_info_cli(file, &beam, &point_forces, &distrib_forces), true);
    
    // Section: { start, end, pointforce, polynomial[] }
    float expected_wrf = 12.0;
    float expected_wrm = -24.0;
    Section expected_raw = { 0, 4.0, 0, { 3.0 } };
    Section expected_shear = { 0, 4.0, 0, { 12.0, -(12.0/4.0)} };

    solveBeam(&beam, 
            point_forces.items, point_forces.count, 
            distrib_forces.items, distrib_forces.count);

    ejtest_expect_float(&R, beam.wall_reaction_force, expected_wrf);
    ejtest_expect_float(&R, beam.wall_reaction_moment, expected_wrm);
    ejtest_expect_struct(&R, beam.raws[0], expected_raw, comp_sections);
    bool r = ejtest_expect_struct(&R, beam.shears[0], expected_shear, comp_sections);

    if (!r)
    {
        printSection(&beam.shears[0]);
        printSection(&expected_shear);
        putchar('\n');
    };

    ejtest_print_result("testExample_6_7", R);
};
void testExample_6_2()
{
    bool R = true;

    FILE * file = fopen(TEST_DIR_NAME"/example_6_2.txt", "r");
    assert(file != NULL);

    Beam beam = {0};
    PointForces point_forces = {0};
    DistributedForces distrib_forces = {0};
    
    ejtest_expect_bool(&R, read_info_cli(file, &beam, &point_forces, &distrib_forces), true);
    
    // Section: { start, end, pointforce, polynomial[] }
    Section expected_raw = { 0, 3.0, 0, { 0, (2.0/3.0) } };
    Section expected_shear = { 0, 3.0, 0, { 3, 0, -(1.0/3.0) } };

    DynamicArrayAppend(&point_forces, (PointForce){ 0 } );

    solveBeam(&beam, 
            point_forces.items, point_forces.count, 
            distrib_forces.items, distrib_forces.count);

    ejtest_expect_float(&R, beam.wall_reaction_force, 3.0);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -4.5);
    ejtest_expect_struct(&R, beam.raws[0], expected_raw, comp_sections);
    ejtest_expect_struct(&R, beam.shears[0], expected_shear, comp_sections);

    ejtest_print_result("testExample_6_2", R);
};
void testReadBeamInput()
{
    bool R = true;

    char line [] = "1.0 10\n";
    Beam beam = {0};
    Beam expected_beam = { 0 };
    expected_beam.length = 1.0;
    expected_beam.sections_count = 10;

    read_beam_info_cli(line, &beam);
    
    ejtest_expect_float(&R, beam.length, expected_beam.length);
    ejtest_expect_int(&R, beam.sections_count, expected_beam.sections_count);

    ejtest_print_result("testReadBeamInput", R);
}
void testReadPointforceInput()
{
    bool R = true;
    bool ret = false;
    PointForce p = {0};

    char line1[] = "0.0  1\n";
    ret = read_pointforce_info_cli(line1, &p);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_float(&R, p.distance, 0.0);
    ejtest_expect_float(&R, p.force, 1.0);

    char line2[] = "0.25 2\n";
    ret = read_pointforce_info_cli(line2, &p);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_float(&R, p.distance, 0.25);
    ejtest_expect_float(&R, p.force, 2.0);

    char line3[] = "0.5  3\n";
    ret = read_pointforce_info_cli(line3, &p);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_float(&R, p.distance, 0.5);
    ejtest_expect_float(&R, p.force, 3.0);

    char line4[] = "1.0  4\n";
    ret = read_pointforce_info_cli(line4, &p);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_float(&R, p.distance, 1.0);
    ejtest_expect_float(&R, p.force, 4.0);

    char line5[] = "1.0                 4\n";
    ret = read_pointforce_info_cli(line5, &p);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_float(&R, p.distance, 1.0);
    ejtest_expect_float(&R, p.force, 4.0);

    ejtest_print_result("testReadPointforceInput", R);
}
void testReadDistribforceInput()
{
    bool R = true;
    bool ret = false;
    DistributedForce d = {0};
    DistributedForce expected_d = {0};

    char line1[] = "0 0.5 [1 0]\n";
    expected_d = (DistributedForce){ .start=0, .end=0.5, .polynomial={1, 0}};
    ret = read_distributedforce_info_cli(line1, &d);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_struct(&R, d, expected_d, comp_distribforces);

    char line2[] = "0.25 0.75 [2 0]\n";
    expected_d = (DistributedForce){ .start=0.25, .end=0.75, .polynomial={2, 0}};
    ret = read_distributedforce_info_cli(line2, &d);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_struct(&R, d, expected_d, comp_distribforces);

    char line3[] = "0.75 1.0 [3 1 2]\n";
    expected_d = (DistributedForce){ .start=0.75, .end=1.0, .polynomial={3, 1, 2, 0}};
    ret = read_distributedforce_info_cli(line3, &d);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_struct(&R, d, expected_d, comp_distribforces);

    char line4[] = "0.65 0.9598709707 [4 0]\n";
    expected_d = (DistributedForce){ .start=0.65, .end=0.9598709707, .polynomial={4, 0}};
    ret = read_distributedforce_info_cli(line4, &d);
    ejtest_expect_bool(&R, ret, true);
    ejtest_expect_struct(&R, d, expected_d, comp_distribforces);

    char line5[] = "0.65 0.95\n";
    ret = read_distributedforce_info_cli(line5, &d);
    ejtest_expect_bool(&R, ret, false);
    
    char line6[] = "";
    ret = read_distributedforce_info_cli(line6, &d);
    ejtest_expect_bool(&R, ret, false);

    ejtest_print_result("testReadDistribforceInput", R);
}
void testReadInput()
{
    bool R = true;

    Beam beam = {0}; 
    Beam expected_beam = {0};
    PointForces point_forces = {0};
    PointForces expected_point_forces = {0};
    DistributedForces distrib_forces = {0};
    DistributedForces expected_distrib_forces = {0};

    char buffer [] = \
	    "#B\n"           \
        "1.0 10\n"          \
        "#PF\n" \
        "0.0  1\n" \
        "0.25 2\n" \
        "0.5  3\n" \
        "1.0  4\n" \
        "#DF\n" \
        "0 0.5 [1 0]\n" \
        "0.25 0.75 [2 0]\n" \
        "0.75 1.0 [3 0]\n" \
        "0.65 0.95 [4 0]\n\n";

	expected_beam.length = 1.0;
	expected_beam.sections_count = 10;

    DynamicArrayAppend(&expected_point_forces, ((PointForce) { 0.0, 1 }));
	DynamicArrayAppend(&expected_point_forces, ((PointForce) { 0.25,2 }));
	DynamicArrayAppend(&expected_point_forces, ((PointForce) { 0.5, 3 }));
	DynamicArrayAppend(&expected_point_forces, ((PointForce) { 1.0, 4 }));

	DynamicArrayAppend(&expected_distrib_forces, ((DistributedForce){ 0, 0.5, {1,0} }));
	DynamicArrayAppend(&expected_distrib_forces, ((DistributedForce){ 0.25, 0.75, {2,0} }));
	DynamicArrayAppend(&expected_distrib_forces, ((DistributedForce){ 0.75, 1.0, {3,0} }));
	DynamicArrayAppend(&expected_distrib_forces, ((DistributedForce){ 0.65, 0.95, {4,0} }));

    assert(expected_point_forces.count == expected_distrib_forces.count);

    FILE * input_stream = fmemopen (buffer, sizeof(buffer), "r");
    read_info_cli(input_stream, &beam, &point_forces, &distrib_forces);
    fclose(input_stream);
    
    ejtest_expect_float(&R, beam.length, expected_beam.length);
    ejtest_expect_int(&R, beam.sections_count, expected_beam.sections_count);

    if (!ejtest_expect_int(&R, point_forces.count, expected_point_forces.count)
            || !ejtest_expect_int(&R, distrib_forces.count, expected_distrib_forces.count))
    {
        ejtest_print_result("testReadInput", R);
        return;
    }

    for (int i = 0; i < expected_point_forces.count; i++)
    {
        ejtest_expect_struct(&R, point_forces.items[i], expected_point_forces.items[i], comp_pointforces);
        ejtest_expect_struct(&R, distrib_forces.items[i], expected_distrib_forces.items[i], comp_distribforces);
    }

    ///////////////////////////////////////////////

    input_stream = fopen (TEST_DIR_NAME"/example_6_2.txt", "r");
    assert(input_stream != NULL);

    beam = (Beam){0}; 
    expected_beam = (Beam){0};
    point_forces.count = 0;
    expected_point_forces.count = 0;
    distrib_forces.count = 0;
    expected_distrib_forces.count = 0;

	expected_beam.length = 3.0;
	expected_beam.sections_count = 4;

	DynamicArrayAppend(&expected_distrib_forces, ((DistributedForce){ 0, 3.0, {0, 0.66667}}));

    read_info_cli(input_stream, &beam, &point_forces, &distrib_forces);
    fclose(input_stream);
    
    ejtest_expect_float(&R, beam.length, expected_beam.length);
    ejtest_expect_int(&R, beam.sections_count, expected_beam.sections_count);

    if (!ejtest_expect_int(&R, point_forces.count, expected_point_forces.count)
            || !ejtest_expect_int(&R, distrib_forces.count, expected_distrib_forces.count))
    {
        ejtest_print_result("testReadInput", R);
        return;
    }

    for (int i = 0; i < expected_point_forces.count; i++)
    {
        ejtest_expect_struct(&R, point_forces.items[i], expected_point_forces.items[i], comp_pointforces);
        ejtest_expect_struct(&R, distrib_forces.items[i], expected_distrib_forces.items[i], comp_distribforces);
    }

    ///////////////////////////////////////////////
    // This example does not have a sections count

    input_stream = fopen (TEST_DIR_NAME"/example_c.txt", "r");
    assert(input_stream != NULL);

    beam = (Beam){0}; 
    expected_beam = (Beam){0};
    point_forces.count = 0;
    expected_point_forces.count = 0;
    distrib_forces.count = 0;
    expected_distrib_forces.count = 0;

	expected_beam.length = 1.0;
	expected_beam.sections_count = MAX_SECTIONS;

	DynamicArrayAppend(&expected_point_forces, ((PointForce){ 1.0, 1.0 }));

    read_info_cli(input_stream, &beam, &point_forces, &distrib_forces);
    fclose(input_stream);
    
    ejtest_expect_float(&R, beam.length, 1.0);
    ejtest_expect_int(&R, beam.sections_count, MAX_SECTIONS);

    if (!ejtest_expect_int(&R, point_forces.count, expected_point_forces.count)
            || !ejtest_expect_int(&R, distrib_forces.count, expected_distrib_forces.count))
    {
        ejtest_print_result("testReadInput", R);
        return;
    }

    for (int i = 0; i < expected_point_forces.count; i++)
    {
        ejtest_expect_struct(&R, point_forces.items[i], expected_point_forces.items[i], comp_pointforces);
        ejtest_expect_struct(&R, distrib_forces.items[i], expected_distrib_forces.items[i], comp_distribforces);
    }
    ejtest_print_result("testReadInput", R);
};
void testCompBeams()
{
    bool R = true;
    Beam a, b;
    a = (Beam){0};
    b = (Beam){0};
    ejtest_expect_bool(&R, comp_beams(&a, &b), true);

    a = (Beam){ .length = 1.0, 0 };
    b = (Beam){ .length = 0.0, 0 };
    ejtest_expect_bool(&R, comp_beams(&a, &b), false);

    a = (Beam){ .length = 1.0, 0 };
    b = (Beam){ .length = 1.0, 0 };
    ejtest_expect_bool(&R, comp_beams(&a, &b), true);

    a = (Beam){ .length = 1.0, .sections_count = 10 };
    b = (Beam){ .length = 1.0, .sections_count = 10 };
    ejtest_expect_bool(&R, comp_beams(&a, &b), true);

    a = (Beam){ .length = 1.0, .sections_count = 10 };
    b = (Beam){ .length = 1.0, .sections_count = 0  };
    ejtest_expect_bool(&R, comp_beams(&a, &b), false);

    ejtest_print_result("testCompBeams", R);
};
void testExample_Empty()
{
    // Given an empty beam and no forces, we should just be able to have a beam
    // with nothing acting on it
    bool R = true;
	Beam beam = {0};

    beam.sections_count = MAX_SECTIONS;

	PointForces point_forces = {0};
	DistributedForces distributed_forces = {0};

    bool solved = solveBeam(   
            &beam, 
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );

    ejtest_expect_bool(&R, solved, true);
    ejtest_expect_int(&R, beam.sections_count, 1);
    ejtest_expect_float(&R, beam.wall_reaction_force, 0);
    ejtest_expect_float(&R, beam.wall_reaction_moment, 0);
    ejtest_expect_float(&R, beam.length, 0);

    ejtest_print_result("testExample_Empty", R);
}
void testExample_A()
{
    bool R = true;
	Beam beam = {0};
	beam.length = 1.0;
	beam.sections_count = MAX_SECTIONS;

	PointForce pointForces[10]; //TODO: change to dynamic array
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
        { .start = 1.000, .end = 0.000, .pointForce = 4.000, .polynomial = {0.00, 0.00, 0.00, 0.00}},
    };
    Section expected_shear[] = {
        { .start = 0.000, .end = 0.250, .pointForce = 0.000, .polynomial = {12.45, -1.00, -0.00, -0.00}},
        { .start = 0.250, .end = 0.500, .pointForce = 0.000, .polynomial = {10.95, -3.00, -0.00, -0.00}},
        { .start = 0.500, .end = 0.650, .pointForce = 0.000, .polynomial = {7.45, -2.00, -0.00, -0.00}},
        { .start = 0.650, .end = 0.750, .pointForce = 0.000, .polynomial = {10.05, -6.00, -0.00, -0.00}},
        { .start = 0.750, .end = 0.950, .pointForce = 0.000, .polynomial = {10.80, -7.00, -0.00, -0.00}},
        { .start = 0.950, .end = 1.000, .pointForce = 0.000, .polynomial = {7.00, -3.00, -0.00, -0.00}},
        { .start = 1.000, .end = 0.000, .pointForce = 0.000, .polynomial = {0.00,  0.00, -0.00, -0.00}},
    };
    Section expected_moment[] = {
        { .start = 0.000, .end = 0.250, .pointForce = 0.000, .polynomial = {-8.24125, 12.45, -0.50, -0.00}},
        { .start = 0.250, .end = 0.500, .pointForce = 0.000, .polynomial = {-7.80375, 10.95, -1.50, -0.00}},
        { .start = 0.500, .end = 0.650, .pointForce = 0.000, .polynomial = {-6.17875, 7.45, -1.00, -0.00}},
        { .start = 0.650, .end = 0.750, .pointForce = 0.000, .polynomial = {-7.02375, 10.05, -3.00, -0.00}},
        { .start = 0.750, .end = 0.950, .pointForce = 0.000, .polynomial = {-7.30500, 10.80, -3.50, -0.00}},
        { .start = 0.950, .end = 1.000, .pointForce = 0.000, .polynomial = {-5.50, 7.00, -1.50, -0.00}},
        { .start = 1.000, .end = 0.000, .pointForce = 0.000, .polynomial = {0.00,  0.00, -0.00, -0.00}},
    };

    ejtest_expect_int(&R, ArrayCount(expected_raw), beam.sections_count);

    for (int i = 0; i < beam.sections_count; i++)
    {
        bool r1, r2, r3;
        r1 = r2 = r3 = true;
        r1 = ejtest_expect_struct(&R, beam.raws[i], expected_raw[i], comp_sections);
        r2 = ejtest_expect_struct(&R, beam.shears[i], expected_shear[i], comp_sections);
        r3 = ejtest_expect_struct(&R, beam.moments[i], expected_moment[i], comp_sections);
        if (!r1 || !r2 || !r3) {
            printf("i: %d\n", i);
        if (!r1)
        {
            printf("Raws:\n");
            printf("%f %f\n", round_to_digits(beam.raws[0].polynomial[0], 6), round_to_digits(expected_raw[0].polynomial[0], 6));
            printSection(&beam.raws[i]);
            printSection(&expected_raw[i]);
            putchar('\n');
        }
        if (!r2)
        {
            printf("Shears:\n");
            printf("%f %f\n", round_to_digits(beam.shears[0].polynomial[0], 6), round_to_digits(expected_shear[0].polynomial[0], 6));
            printSection(&beam.shears[i]);
            printSection(&expected_shear[i]);
            putchar('\n');
        }
        if (!r3)
        {
            printf("Moments:\n");
            printf("%f %f\n", round_to_digits(beam.moments[0].polynomial[0], 6), round_to_digits(expected_moment[0].polynomial[0], 6));
            printSection(&beam.moments[i]);
            printSection(&expected_moment[i]);
            putchar('\n');
        }
        }
    };

    ejtest_print_result("testExample_A", R);
}
void testExample_B()
{
    bool R = true;
	Beam beam = {0};

    beam.length = 1.0;
    beam.sections_count = MAX_SECTIONS;

	PointForces point_forces = {0};
	DistributedForces distributed_forces = {0};

    DynamicArrayAppend(&point_forces, ((PointForce){ .distance = 1, .force = 1 }));

    bool solved = solveBeam(   
            &beam, 
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );
    ejtest_expect_bool(&R, solved, true);

    ejtest_expect_int(&R, beam.sections_count, 2);
    ejtest_expect_float(&R, beam.wall_reaction_force, 1);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -1);

    ejtest_print_result("testExample_B", R);
};
void testExample_C()
{
    bool R = true;
    const char * ejtest_test_name = "testExample_C";
    {

	Beam beam = {0};
    beam.length = 1.0;
    beam.sections_count = MAX_SECTIONS;

	PointForces point_forces = {0};
	DistributedForces distributed_forces = {0};

    DynamicArrayAppend(&distributed_forces, ((DistributedForce){ .start = 0.5, .end = 1, .polynomial = {2,0}}));

    bool solved = solveBeam(   
            &beam, 
            point_forces.items, point_forces.count,
            distributed_forces.items, distributed_forces.count
            );
    ejtest_expect_bool(&R, solved, true);

    ejtest_expect_int(&R, beam.sections_count, 3);
    ejtest_expect_float(&R, beam.wall_reaction_force, 1);
    ejtest_expect_float(&R, beam.wall_reaction_moment, -1*0.75);
    }

    ejtest_print_result(ejtest_test_name, R);
};
void testWallReactionForce()
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
void testWallReactionMoment()
{
    bool R = true;
    float wrm;
    Section sections[2] = {0};

    sections[0] = (Section){0};
    sections[1] = (Section){ .start = 1, .pointForce = 1 };
    wrm = calculateWallReactionMoment(sections, 2);
    ejtest_expect_float(&R, wrm, -1.0);

    sections[0] = (Section){0};
    sections[1] = (Section){ .start = 0, .end = 1, .polynomial = {1,0} };
    wrm = calculateWallReactionMoment(sections, 2);
    ejtest_expect_float(&R, wrm, -0.5);

    ejtest_print_result("testWallReactionMoment", R);
}
void testSeperateSections()
{
	//TODO("Write more section tests");
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
		(Section){ 0.000000, 0.250000, 1.000000 , {1.00, 0.00, 0.00, 0.00}},
		(Section){ 0.250000, 0.500000, 2.000000 , {3.00, 0.00, 0.00, 0.00}},
		(Section){ 0.500000, 0.650000, 3.000000 , {2.00, 0.00, 0.00, 0.00}},
		(Section){ 0.650000, 0.750000, 0.000000 , {6.00, 0.00, 0.00, 0.00}},
		(Section){ 0.750000, 0.950000, 0.000000 , {7.00, 0.00, 0.00, 0.00}},
		(Section){ 0.950000, 1.000000, 0.000000 , {3.00, 0.00, 0.00, 0.00}},
		(Section){ 1.000000, 0.000000, 4.000000 , {0.00, 0.00, 0.00, 0.00}},
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
