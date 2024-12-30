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
	float pointForce;
	float polynomial[MAX_POLYNOMIAL_DEGREE];
};

typedef struct Section Section;

// printSection
void printSection(const void * vp)
{
	Section * s = (Section *)vp;
	printf("Section:: start: %f, end: %f, pointForce: %f, coeff0: %f\n", s->start, s->end, s->pointForce, s->polynomial[0]);
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
void seperateBeamIntoSections(float beamLength, PointForce pForces[], 		int pfCount, DistributedForce dForces[], 	int dfCount, Section sections[], 		int * sectionsCount)
{

	PointForce * pF = pForces;
	// Make an array of pointers so we can sort them based on ends vs starts
	DistributedForce ** dFS = malloc(dfCount * sizeof(DistributedForce *));
	DistributedForce ** dFE = malloc(dfCount * sizeof(DistributedForce *));

	for ( int i = 0; i < dfCount; i++ )
	{
		dFS[i] = &dForces[i];
		dFE[i] = &dForces[i];
	}

	// Sorting lets us use a cool trick, narrows down the checks we have to
	// do since we know which shoud come next
	// With this, we can keep a current index for each array and then move
	// it up once we "use" it
	qsort( pF , pfCount, sizeof (PointForce), compPointDists );
	qsort( dFS, dfCount, sizeof (DistributedForce *), compDistributedStartsPtr );
	qsort( dFE, dfCount, sizeof (DistributedForce *), compDistributedEndsPtr );

	int iDS = 0, iDE = 0, iPF = 0; // index of dFS, dFE, pF
	int iSection = 0;
	LL_Node * head = NULL;

	printStructArray(pF, pfCount, sizeof(PointForce), printPF);
	printStructArray(dFS, dfCount, sizeof(DistributedForce*), printDFptr);

	while (iDS < dfCount || iDE < dfCount || iPF < pfCount)
	{

		// Note: these booleans are a little janky but we need them
		// like this to ensure that we are always only checking valid
		// pointers and so we can keep a nice and simple if else flow
		// control
		//
		// A better approach would be appreciated
		
		int P_less_S = (iDS < dfCount) ? pF[iPF].distance < dFS[iDS]->start : 1;
		int P_less_E = (iDE < dfCount) ? pF[iPF].distance < dFE[iDE]->end : 1;
		
		int S_less_E = iDS < dfCount && ((iDE < dfCount) ? dFS[iDS]->start < dFE[iDE]->end : 1);
		if (iPF < pfCount && P_less_S && P_less_E)
		{
			// POINT FORCE
			// Create new section
			int x = !nearlyEqual(pF[iPF].distance, sections[iSection].start);
			if (x)
			{
			sections[iSection].end = pF[iPF].distance;

			// Sum up linked list
			LL_SumDistributedPolynomials(head, sections[iSection].polynomial);

			iSection++;
			sections[iSection].start = pF[iPF].distance;
			}

			// Save force in section
			sections[iSection].pointForce = pF[iPF].force;
			iPF++;
		} else if (iDS < dfCount && S_less_E)
		{
			// START
			// Create new section
			if (!nearlyEqual(dFS[iDS]->start, sections[iSection].start))
			{
			sections[iSection].end = dFS[iDS]->start;

			// Sum up linked list
			LL_SumDistributedPolynomials(head, sections[iSection].polynomial);

			iSection++;
			sections[iSection].start = dFS[iDS]->start;
			}

			// Push force to linked list
			LL_push(&head, dFS[iDS]);
			iDS++;
		} else
		{
			// END
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
}

	// make sure sections cover only/entirely the beam
	if (sections[iSection].start < beamLength) sections[iSection].end = beamLength;
	else sections[iSection-1].end = beamLength;

	*sectionsCount = iSection;

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
