/*
* Filename:	main.c
* Date:		24/12/2024 
* Name:		EL Joubert
*
* SOMP
* Made to simulate the stress in a beam caused by simple forces and display the
* functions that these forces produce
*
* will support:
* - point forces
* - distributed forces in the form of polynomials of x^n where n >= 0
* will NOT support:
* - point moments
* - distributed moments? (if that is even physically possible???)
*
*/

#define TODO(msg) printf("[%s, %d] TODO: "msg"\n",__FILE__, __LINE__);

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UTILS_IMPLEMENTATION
#include "utils.h" // somp_logic.h depends on this so it should go first

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

#define SOMP_IO_IMPLEMENTATION
#include "somp_io.h"

int main(int argc, char * argv[])
{
	printf("Welcome to SOMP!\n");
	printf("SOMP calculates the internal shear stress and bending moments in cantilever beams\n");
    printf("Formatting rules:\n");
    printf("\tNormal text - write it exactly as shown\n");
    printf("\t(Bracketed) - descriptions, do NOT write\n");
    printf("\t[Squared  ] - must write, but use your own values\n");
    printf("Example input:\n");
	printf("\t#B\n");
    printf("\t1.0 10\n");
    printf("\t#PF\n");
    printf("\t4\n");
    printf("\t0.0  1\n");
    printf("\t0.25 2\n");
    printf("\t0.5  3\n");
    printf("\t1.0  4\n");
    printf("\t#DF\n");
    printf("\t4\n");
    printf("\t0 0.5 [1 0]\n");
    printf("\t0.25 0.75 [2 0]\n");
    printf("\t0.75 1.0 [3 0]\n");
    printf("\t0.65 0.95 [4 0]\n");

    Beam beam = {0};
    PointForces point_forces = {0};
    DistributedForces distrib_forces = {0};
    //input
    printf("Enter your input:\n");
    //TODO: when pressing ^D, it does an infinite loop of failure, this shouldnt happen
    while (true)
    {
        if (read_info_cli(stdin, &beam, &point_forces, &distrib_forces)) break;
        printf("\nCould not parse input! Please type it again.\n");
        printf("Enter your input:\n");
    };
    solveBeam(&beam, point_forces.items, point_forces.count, distrib_forces.items, distrib_forces.count);
    printf("\nOutput:\n");

    printStructArray(beam.raws, beam.sections_count, sizeof(beam.raws[0]), printSection );
    printStructArray(beam.shears, beam.sections_count, sizeof(beam.shears[0]), printSection );
    printStructArray(beam.moments, beam.sections_count, sizeof(beam.moments[0]), printSection );

	return 0;
}
