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
#include "utils.c"

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

// printSection
void printSection(const void * vp)
{
	Section * s = (Section *)vp;
	printf("Section:: start: %f, end: %f, coeff0: %f\n", s->start, s->end, s->polynomial[0]);
}
// printPointForce
void printPF(const void * vp)
{
	PointForce * p = (PointForce *) vp;
	printf("PointForce:: distance: %f, force: %f\n", p->distance, p->force);
}

// print a DistributedForce
void printDF(const void * vd)
{
	DistributedForce * d = (DistributedForce *) vd;
	printf("DistributedForce:: start: %f, end: %f, coeff0: %f\n", d->start, d->end, d->polynomial[0]);
}

// print a DistributedForce from pointer
void printDFptr(const void * vd)
{
	DistributedForce ** d = (DistributedForce **) vd;
	printf("DistributedForce:: start: %f, end: %f, coeff0: %f\n", (*d)->start, (*d)->end, (*d)->polynomial[0]);
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

void LL_SumDistributedPolynomials(LL_Node * head, float poly[])
{
	LL_Node * current = head;
	while (current)
	{
		DistributedForce * d = (DistributedForce *) current->data;
		for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
		{
			poly[i] += d->polynomial[i];
		}
		current = current->next;
	}
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

int compDistributedStartsPtr(const void * a, const void * b)
{
	DistributedForce ** adp = (DistributedForce **) a;
	DistributedForce ** bdp = (DistributedForce **) b;
	DistributedForce * ad = *adp;
	DistributedForce * bd = *bdp;

	if (ad->start < bd->start) return -1;
	else if (ad->start > bd->start) return 1;
	
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

int compDistributedEndsPtr(const void * a, const void * b)
{
	DistributedForce ** adp = (DistributedForce **) a;
	DistributedForce ** bdp = (DistributedForce **) b;
	DistributedForce * ad = *adp;
	DistributedForce * bd = *bdp;

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
	/* dForces[0] = (DistributedForce){ 0.25, 0.75, {2,0} }; */
	/* dForces[1] = (DistributedForce){ 0, 0.5, {1,0} }; */
	/* dfCount = 2; */

	// Make an array of pointers
	DistributedForce ** dFS = malloc(dfCount * sizeof(DistributedForce *));
	DistributedForce ** dFE = malloc(dfCount * sizeof(DistributedForce *));

	for ( int i = 0; i < dfCount; i++ )
	{
		dFS[i] = &dForces[i];
		dFE[i] = &dForces[i];
	}

	/* qsort( pF , pfCount, sizeof (PointForce), compPointDists ); */
	qsort( dFS, dfCount, sizeof (DistributedForce *), compDistributedStartsPtr );
	qsort( dFE, dfCount, sizeof (DistributedForce *), compDistributedEndsPtr );

	TODO("Split into sections");
	int iDS = 0, iDE = 0, iPF = 0; // index of dFS, dFE, pF
	int iSection = 0;
	LL_Node * head = NULL;
	while (iDS < dfCount || iDE < dfCount)
	{
		if (iDS < dfCount && dFS[iDS]->start < dFE[iDE]->end)
		{
			// Create new section
			sections[iSection].end = dFS[iDS]->start;

			// Sum up linked list
			LL_SumDistributedPolynomials(head, sections[iSection].polynomial);

			iSection++;
			sections[iSection].start = dFS[iDS]->start;

			// Push force to linked list
			LL_push(&head, dFS[iDS]);
			iDS++;
		} else 
		{
			// Create new section
			sections[iSection].end = dFE[iDE]->end;

			// Sum up linked list
			LL_SumDistributedPolynomials(head, sections[iSection].polynomial);

			iSection++;
			sections[iSection].start = dFE[iDE]->end;
			// Remove force from linked list
			LL_remove(&head, dFE[iDE]);
			iDE++;
		}
		LL_print(head, printDF);
	}

	// make sure sections cover only/entirely the beam
	if (sections[iSection].start < beamLength) sections[iSection].end = beamLength;
	else sections[--iSection].end = beamLength;

	sections++; //Discard the first struct because it goes from 0 to 0, hacky way of doing it
	printStructArray(sections, iSection, sizeof(Section), printSection);

	*sectionsCount = 1;

	free(dFS);
	free(dFE);
	LL_free(head);
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
