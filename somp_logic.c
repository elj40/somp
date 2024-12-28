/*
* Filename:	somp_logic.c
* Date:		24/12/2024 
* Name:		EL Joubert
*
* module that contains the math part of somp, stuff like solving the beam
* stresses
*/
#define MAX_POLYNOMIAL_DEGREE 3
#include <stdlib.h>
#include <string.h>

struct PointForce 
{
	float distance;
	float force;
};

typedef struct PointForce PointForce;

struct DistributedForce
{
	float start;
	float end;
	float polynomial[MAX_POLYNOMIAL_DEGREE];
};

typedef struct DistributedForce DistributedForce;

struct Section
{
	float start;
	float end;
	float polynomial[MAX_POLYNOMIAL_DEGREE];
};

typedef struct Section Section;

// printPointForce
void printPF(const void * vp)
{
	PointForce * p = (PointForce *) vp;
	printf("PointForce -> distance: %f, force: %f\n", p->distance, p->force);
}

// print a DistributedForce
void printDF(const void * vd)
{
	DistributedForce * d = (DistributedForce *) vd;
	printf("DistributedForce -> start: %f, end: %f, coeff0: %f\n", d->start, d->end, d->polynomial[0]);
}

void printStructArray(const void * arr, int count, int size, void (* printStruct)(const void *) )
{
	printf("[\n");
	for (int i = 0; i < count; i++)
	{
		printf("  ");
		const void * p = arr + i * size;
		(*printStruct)(p);
	}
	printf("]\n");
}
	

/* comparison function which returns a negative integer value if the
 * first argument is less than the second, a positive integer value if the
 * first argument is greater than the second and zero if the arguments are
 * equivalent. */
int compPointDists(const void * a, const void * b)
{
	PointForce * ap = (PointForce *) a;
	PointForce * bp = (PointForce *) b;
	if (ap->distance < bp->distance) return -1;
	else if (ap->distance > bp->distance) return 1;
	
	return 0;
}
int compDistributedStarts(const void * a, const void * b)
{
	DistributedForce * ad = (DistributedForce *) a;
	DistributedForce * bd = (DistributedForce *) b;
	if (ad->start < bd->start) return -1;
	else if (ad->start > bd->start) return 1;
	
	return 0;
}
int compDistributedEnds(const void * a, const void * b)
{
	DistributedForce * ad = (DistributedForce *) a;
	DistributedForce * bd = (DistributedForce *) b;
	if (ad->end < bd->end) return -1;
	else if (ad->end > bd->end) return 1;
	
	return 0;
}
void seperateBeamIntoSections(float beamLength, 
		PointForce pForces[], 		int pfCount, 
		DistributedForce dForces[], 	int dfCount, 
		Section sections[], 		int * sectionsCount)
{
	// First test input
	/* beamLength = 1.0; */
	/* dForces[1] = (DistributedForce){ 0, 0.5, {1,0} }; */
	/* dForces[2] = (DistributedForce){ 0.25, 0.75, {2,0} }; */

	PointForce * pF = pForces;
	DistributedForce * dFS = dForces;
	DistributedForce * dFE = malloc(dfCount * sizeof(DistributedForce));
	memcpy( dFE, dForces, dfCount * sizeof(DistributedForce) );

	printStructArray(dForces, 2, sizeof(DistributedForce), printDF);

	TODO("Sort each array");
	/* qsort( (void*) pF , (size_t) pfCount, sizeof (PointForce), compPointDists ); */
	/* qsort( (void*) dFS, (size_t) dfCount, sizeof (DistributedForce), compDistributedStarts ); */
	/* qsort( (void*) dFE, (size_t) dfCount, sizeof (DistributedForce), compDistributedEnds ); */
	
	TODO("Split into sections");
	*sectionsCount = 1;

	free(dFE);
}

int calculateWallReactionForce(PointForce pForces[], int pfCount, DistributedForce dForces[], int dfCount)
{
	TODO("calculate wall reaction");
	return 0;
}

float evalPolynomial(float poly[])
{
	TODO("evaluate polynomial");
	return 0.0;
}

void integratePolynomial(float poly[])
{
	TODO("integrate a polynomial");
}

void solveBeam()
{
	float beamLength = 3.0;
	Section sections[20] = {0};
	int sectionsCount;

	PointForce pointForces[1] = {0}; int pfCount = 0;
	DistributedForce distributedForces[1] = {0}; int dfCount = 0;
	// Seperate beam into sections
	seperateBeamIntoSections(beamLength, pointForces, pfCount, distributedForces, dfCount, sections, &sectionsCount);

	// Solve global wall force (first sections startForce)
	sections[0].polynomial[0] = calculateWallReactionForce(pointForces, pfCount, distributedForces, dfCount);

	// For every section:
	// 	integrate every sections polynomial (if it exists)
	// 	find c-value of every section (startForce)
	// 	store section in an array
	for ( int i = 0; i < sectionsCount; i++ )
	{
		if (sections[i].polynomial) integratePolynomial(sections[i].polynomial);
		if (i > 0) sections[i].polynomial[0] = evalPolynomial(sections[i-1].polynomial) - evalPolynomial(sections[i].polynomial);
	}
}
