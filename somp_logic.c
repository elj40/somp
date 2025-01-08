/*
* Filename:	somp_logic.c
* Date:		24/12/2024 
* Name:		EL Joubert
*
* module that contains the math part of somp, stuff like solving the beam
* stresses
*/
#define MAX_POLYNOMIAL_DEGREE 4
#include <stdlib.h>
#include <string.h>
#include "utils.c"

void integratePolynomial(float dest[MAX_POLYNOMIAL_DEGREE], const float src[MAX_POLYNOMIAL_DEGREE]);
float evalPolynomial(float x, float poly[]);

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

int compSections(Section a, Section b) 
{
	if (!nearlyEqual(a.start, b.start)) return 0;
	if (!nearlyEqual(a.end, b.end)) return 0;
	if (!nearlyEqual(a.pointForce, b.pointForce)) return 0;

	return 1;
}

void printSection(const void * vp)
{
	Section * s = (Section *)vp;
	printf("Section:: start: %.3f, end: %.3f, pointForce: %.3f, poly: [", s->start, s->end, s->pointForce);
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		printf("%.2f", s->polynomial[i]);
		if (i < MAX_POLYNOMIAL_DEGREE-1) printf(", ");
	}
	printf("]\n");
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
		PointForce pForces[],       int pfCount, 
		DistributedForce dForces[], int dfCount, 
		Section sections[],         int * sectionsCount)
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

void printPolynomial(float p[MAX_POLYNOMIAL_DEGREE])
{
	printf("[ ");
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		printf("%.1f", p[i]);
		if (i < MAX_POLYNOMIAL_DEGREE-1 ) printf(", ");
	}
	printf(" ]");

}

float calculateWallReactionMoment(Section sections[], int sectionsCount)
{
	float pointSum = 0;
	float distributedSum = 0;
	float integrated[MAX_POLYNOMIAL_DEGREE];

	for ( int i = 0; i < sectionsCount; i++ )
	{
		pointSum += sections[i].pointForce * sections[i].start;
		
		integratePolynomial(integrated, sections[i].polynomial);
		float integral = evalPolynomial(sections[i].end, integrated) - evalPolynomial(sections[i].start, integrated);
		distributedSum += integral * (sections[i].start + sections[i].end)/2;
	}
	return pointSum + distributedSum;
}

float calculateWallReactionForce(Section sections[], int sectionsCount)
{
	// Prefer to do calculate wall reaction force using sections because it
	// ensures that we dont consider forces longer than the beam
	/* TODO("calculate wall reaction"); */

	float pointSum = 0;
	float distributedSum = 0;
	float integrated[MAX_POLYNOMIAL_DEGREE];

	for (int i = 0; i < sectionsCount; i++)
	{
		pointSum += sections[i].pointForce;
		integratePolynomial(integrated, sections[i].polynomial);

		distributedSum += evalPolynomial(sections[i].end, integrated) - evalPolynomial(sections[i].start, integrated);
	}
	return pointSum + distributedSum;
}

float evalPolynomial(float x, float poly[MAX_POLYNOMIAL_DEGREE])
{
	float answer = 0;
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		answer += poly[i] * pow(x, i);
	}

	return answer;
}

void integratePolynomial(float dest[MAX_POLYNOMIAL_DEGREE], const float src[MAX_POLYNOMIAL_DEGREE])
{
	// NOTE: if the src polynomial has a degree of MAX_POLYNOMIAL_DEGREE,
	// that term will not be able to be integrated
	// Make sure that MAX_POLYNOMIAL_DEGREE is always one higher than the
	// highest degree in src
	for (int i = 1; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		dest[i] = src[i-1]/(float)i;
	}
	dest[0] = 0.0f;
}

void solveBeam()
{
	Section rawSections[20] = {0};
	Section shearSections[20] = {0};
	Section momentSections[20] = {0};

	float beamLength = 1.0;
	int pfCount = 0;
	int dfCount = 0;
	int sectionsCount = 0;

	PointForce       pointForces[5]   = {0}; 
	DistributedForce distributedForces[5]   = {0};
	Section          sections[20] = {0};
	beamLength = 1.0;
	pointForces[0] = (PointForce){ 0.0, 1 };
	pointForces[1] = (PointForce){ 0.25,2 };
	pointForces[2] = (PointForce){ 0.5, 3 };
	pointForces[3] = (PointForce){ 1.0, 4 };
	pfCount = 4;
	distributedForces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	distributedForces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	distributedForces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	distributedForces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;

	seperateBeamIntoSections(beamLength, pointForces, pfCount, distributedForces, dfCount, rawSections, &sectionsCount);

	// Solve global wall force (first sections startForce) 
	float wallReactionForce = calculateWallReactionForce(rawSections, sectionsCount);
	float wallReactionMoment = -calculateWallReactionMoment(rawSections, sectionsCount);

	// Shear section calculations
	for ( int i = 0; i < sectionsCount; i++ )
	{
		shearSections[i].start = rawSections[i].start;
		shearSections[i].end = rawSections[i].end;
		integratePolynomial(shearSections[i].polynomial, rawSections[i].polynomial);

		//Multiply the integral by -1 because in this context the
		//distributed force would cause counterclockwise rotation
		for (int j = 0; j < MAX_POLYNOMIAL_DEGREE; j++) shearSections[i].polynomial[j] *= -1;

		// Solve for c
		// end of previous = start of current + c - current point force
		// => c = end of p - start of current + current pf

		if (i == 0) shearSections[i].polynomial[0] = wallReactionForce - rawSections[0].pointForce;
		else
		{
			float previous = evalPolynomial(shearSections[i-1].end, shearSections[i-1].polynomial);
			float current = evalPolynomial(shearSections[i].start, shearSections[i].polynomial);
			float point_force = rawSections[i].pointForce;
			shearSections[i].polynomial[0] =  previous - current - point_force;
		}

	}
	
	printf("Raw:\n");
	printStructArray(rawSections, sectionsCount, sizeof(Section), printSection);
	printf("Shear:\n");
	printStructArray(shearSections, sectionsCount, sizeof(Section), printSection);

	// Moment sections calculations
	for ( int i = 0; i < sectionsCount; i++ )
	{
		momentSections[i].start = shearSections[i].start;
		momentSections[i].end = shearSections[i].end;

		integratePolynomial(momentSections[i].polynomial, shearSections[i].polynomial);

		//do not need to multiply by negative one here

		if (i == 0) momentSections[i].polynomial[0] = wallReactionMoment;
		else
		{
			float previous = evalPolynomial(momentSections[i-1].end, momentSections[i-1].polynomial);
			float current = evalPolynomial(momentSections[i].start, momentSections[i].polynomial);
			// TODO: consider point moments
			momentSections[i].polynomial[0] =  previous - current;
		}
	}

	printf("Moment:\n");
	printStructArray(momentSections, sectionsCount, sizeof(Section), printSection);
}
