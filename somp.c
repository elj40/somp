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


int main(int argc, char * argv[])
{
	printf("Hello SOMP\n");

    // Get beam data from user
    //      beam length
    //      point forces
    //      distributed forces
	Beam beam = {0};
	beam.length = 1.0;
	beam.sectionsCount = MAX_SECTIONS;

	PointForce point_forces[10]; //TODO: figure out how many we should allocate for
	int pfCount;
	point_forces[0] = (PointForce){ 0.0, 1 };
	point_forces[1] = (PointForce){ 0.25,2 };
	point_forces[2] = (PointForce){ 0.5, 3 };
	point_forces[3] = (PointForce){ 1.0, 4 };
	pfCount = 4;

	DistributedForce distributed_forces[10];
	int dfCount;
	distributed_forces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	distributed_forces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	distributed_forces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	distributed_forces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;

    read_beam_info(&beam);
    read_pointforce_info(point_forces);
    read_distributedforce_info(distributed_forces);
    // Solve beam
	solveBeam(&beam, point_forces, pfCount, distributed_forces, dfCount);

    // Ouput beam results

	return 0;
}
