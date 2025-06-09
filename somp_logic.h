#ifndef SOMP_LOGIC_H
#define SOMP_LOGIC_H

/*
* Filename:	somp_logic.c
* Date:		24/12/2024 
* Name:		EL Joubert
*
* module that contains the math part of somp, stuff like solving the beam
* stressed
*/

#include <stdbool.h>

#define UTILS_IMPLEMENTATION
#include "utils.h"

#define MAX_POLYNOMIAL_DEGREE 4
#define MAX_SECTIONS 20

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

typedef struct PointForces PointForces;
typedef struct DistributedForces DistributedForces;

struct PointForces {
    PointForce * items;
    int count;
    int capacity;
};

struct DistributedForces{
    DistributedForce * items;
    int count;
    int capacity;
};

struct Section
{
	float start;
	float end;
	float pointForce;
	float polynomial[MAX_POLYNOMIAL_DEGREE];
};
typedef struct Section Section;

struct Beam {
	float length;
    float wall_reaction_force;
    float wall_reaction_moment;
	int sections_count;
	Section raws[MAX_SECTIONS];
	Section shears[MAX_SECTIONS];
	Section moments[MAX_SECTIONS];
};
typedef struct Beam Beam;

float evalPolynomial(float x, float poly[MAX_POLYNOMIAL_DEGREE]);
void integratePolynomial(float dest[MAX_POLYNOMIAL_DEGREE], const float src[MAX_POLYNOMIAL_DEGREE]);

void printSection(const void * vp);
void printPF(const void * vp);
void printDF(const void * vd);
void printDFptr(const void * vd);
void printStructArray(const void * arr, int count, int size, void (* printStruct)(const void *) );
void printPolynomial(float p[MAX_POLYNOMIAL_DEGREE]);

void LL_SumDistributedPolynomials(LL_Node * head, float poly[]);

bool comp_sections(void * a, void * b);
int compSections(Section a, Section b);
int compPointDists(const void * a, const void * b);
int compDistributedStartsPtr(const void * a, const void * b);
int compDistributedStarts(const void * a, const void * b);
int compDistributedEnds(const void * a, const void * b);
int compDistributedEndsPtr(const void * a, const void * b);

bool seperateBeamIntoSections(float beamLength,
		PointForce pForces[],       int pfCount, 
		DistributedForce dForces[], int dfCount, 
		Section sections[],         int * sectionsCount);
float calculateWallReactionMoment(Section sections[], int sectionsCount);
float calculateWallReactionForce(Section sections[], int sectionsCount);

void solveShearSections(Section shear[], Section raw[], int count);
void solveMomentSections(Section moment[], Section shear[], Section raw[], int count);
bool solveBeam(Beam * beam,
		PointForce pointForces[], int pfCount,
		DistributedForce distributedForces[], int dfCount);

#ifdef SOMP_LOGIC_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>  
#include <string.h>
#include <stdbool.h>
#include <math.h>

/// FROM STACKOVERFLOW: Daniel Gehriger at 
/// https://stackoverflow.com/questions/13094224/a-c-routine-to-round-a-float-to-n-significant-digits
double round_to_digits(double value, int digits)
{
    if (value == 0.0) // otherwise it will return 'nan' due to the log10() of zero
        return 0.0;

    double factor = pow(10.0, digits - ceil(log10(fabs(value))));
    return round(value * factor) / factor;   
}

bool comp_beams(void * a, void * b) 
{
    Beam * A = (Beam *) a;
    Beam * B = (Beam *) b;
	if (!nearly_equal(A->length, B->length)) { return false;}
	if (A->sections_count != B->sections_count) { return false;}

    for (int j = 0; j < A->sections_count; j++)
    {
        if (!comp_sections(&A->raws[j], &B->raws[j])) return false;
        if (!comp_sections(&A->shears[j], &B->shears[j])) return false;
        if (!comp_sections(&A->moments[j], &B->moments[j])) return false;
    }

	return true;
}

bool comp_sections(void * a, void * b) 
{
    Section * A = (Section *) a;
    Section * B = (Section *) b;
	if (!nearly_equal(A->start, B->start)) { printf("1\n"); return false; };
	if (!nearly_equal(A->end, B->end)) { printf("2\n"); return false; };
	if (!nearly_equal(A->pointForce, B->pointForce)) { printf("3\n"); return false; };

    for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
    {
        float term_A = round_to_digits(A->polynomial[i], 6);
        float term_B = round_to_digits(B->polynomial[i], 6);
        if (!nearly_equal(term_A, term_B)) { printf("4\n"); return false; };
    }

	return true;
}

bool comp_pointforces(void * a, void * b) 
{
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    PointForce * A = (PointForce *) a;
    PointForce * B = (PointForce *) b;
	if (!nearly_equal(A->distance, B->distance)) return false;
	if (!nearly_equal(A->force, B->force)) return false;

	return true;
}

bool comp_distribforces(void * a, void * b) 
{
    if (a == NULL && b == NULL) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    DistributedForce * A = (DistributedForce *) a;
    DistributedForce * B = (DistributedForce *) b;
	if (!nearly_equal(A->start, B->start)) return false;
	if (!nearly_equal(A->end, B->end)) return false;


    for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
    {
        if (!nearly_equal(A->polynomial[i], B->polynomial[i])) return false;
    }

	return true;
}

void printSection(const void * vp)
{
    const int decimals = 3;
    const int poly_decimals = 4;
	Section * s = (Section *)vp;
    printf("Section:: start: %.*f, end: %.*f, pointForce: %.*f, poly: [",
            decimals, s->start, 
            decimals, s->end, 
            decimals, s->pointForce);
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		printf("%.*f", poly_decimals, s->polynomial[i]);
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
	printf("DistributedForce:: start: %.3f, end: %.3f, poly: [", d->start, d->end);
    for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
    {
        printf(" %.3f", d->polynomial[i]);
        if (i < MAX_POLYNOMIAL_DEGREE-1) printf(",");
    };
    printf("]\n");
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
/*
* Seperate beam into sections
* Converts an array of point forces and potentially overlapping distributed
* forces into an array of sections that cover the entire beam
*
* beamLength: length of the beam, used to cut off sections that go too far or add an "empty" section
* pForces: array of point forces to be used
* pfCount: number of elements in pForces
* dForces: array of distributed forces to be used
* dfCount: number of elements in dForces
* sections: a buffer array that will be filled with Section objects
* sectionsCount: pointer to a variable the holds the max amount of sections
* 	allowed to be stored, if the number of sections found exceeds this number,
* 	it will return false. After all the sections have been found sectionsCount
* 	is updated to be the amount of sections in the sections array
*/
bool seperateBeamIntoSections(float beamLength,
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
        //
        // BIG WHOOPS HERE!!!
        // We still get a segfault if there are zero point forces 
		
        int P_less_S = 0; 
        int P_less_E = 0;

        if (pfCount != 0)
        {
            P_less_S = (iDS < dfCount) ? pF[iPF].distance < dFS[iDS]->start : 1;
            P_less_E = (iDE < dfCount) ? pF[iPF].distance < dFE[iDE]->end : 1;
        }
		
		int S_less_E = iDS < dfCount && ((iDE < dfCount) ? dFS[iDS]->start < dFE[iDE]->end : 1);
		if (iPF < pfCount && P_less_S && P_less_E)
		{
			// POINT FORCE
			// Create new section
			int x = !nearly_equal(pF[iPF].distance, sections[iSection].start);
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
			if (!nearly_equal(dFS[iDS]->start, sections[iSection].start))
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

    // This is to make sure we do not exceed the amount memory allocated, but
    // since we plan on using dynamic arrays it should become obsolete
	if (iSection >= *sectionsCount) return false;

	// make sure sections cover only/entirely the beam
	if (sections[iSection].start < beamLength) sections[iSection].end = beamLength;
	else sections[iSection-1].end = beamLength;

    // Since iSection represents the index of the last section, the no. of
    // sections is iSection + 1
	*sectionsCount = iSection+1;

	free(dFS);
	free(dFE);
	LL_free(head);

	return true;
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

/*
 * Calculate the reaction moment of the wall, will return a negative value when
 * the wall is on the left and forces are pushing down on the beam
 *
 * Parameters:
 *  [in] Section sections: raw sections to calculate moment about
 *  [in] int sectionsCount: number of sections
 *
 * Return:
 *  float: reaction moment of wall
 */
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
	return -(pointSum + distributedSum);
}

float calculateWallReactionForce(Section sections[], int sectionsCount)
{
	// Prefer to do calculate wall reaction force using sections because it
	// ensures that we dont consider forces longer than the beam

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

// NOTE: there is a lot of overlap between solveShearSections and
// solveMomentSections, would be good to find a way to generalize this a bit
void solveShearSections(Section shear[], Section raw[], int count)
{
	float wallReactionForce = calculateWallReactionForce(raw, count);

	for (int i = 0; i < count; i++)
	{
		shear[i].start = raw[i].start;
		shear[i].end = raw[i].end;

		integratePolynomial(shear[i].polynomial, raw[i].polynomial);

		for (int j = 0; j < MAX_POLYNOMIAL_DEGREE; j++) shear[i].polynomial[j] *= -1;

		if (i == 0) shear[i].polynomial[0] = wallReactionForce - raw[0].pointForce;
		else 
		{
			float previous = evalPolynomial(shear[i-1].end, shear[i-1].polynomial);
			float current = evalPolynomial(shear[i].start, shear[i].polynomial);
			float point = raw[i].pointForce;
			shear[i].polynomial[0] =  previous - current - point;
		}
	}
}

void solveMomentSections(Section moment[], Section shear[], Section raw[], int count)
{
	float wallReactionMoment = -calculateWallReactionMoment(raw, count);

	for (int i = 0; i < count; i++)
	{
		moment[i].start = shear[i].start;
		moment[i].end = shear[i].end;
 
 		integratePolynomial(moment[i].polynomial, shear[i].polynomial);

		if (i == 0) moment[i].polynomial[0] = -wallReactionMoment;
		else 
		{
			float previous = evalPolynomial(moment[i-1].end, moment[i-1].polynomial);
			float current = evalPolynomial(moment[i].start, moment[i].polynomial);
			//TODO: make point moments
			moment[i].polynomial[0] =  previous - current;
		}
	}
}
/* 
 * Solve for the shear and moment sections of the beam
 *
 * Parameters:
 *  [out]beam: pointer to beam whose sections will get modified
 *  [in]pointForces[]: array of point forces acting on beam
 *  [in]pfCount: number of pointforces
 *  [in]distributedForces[]: array of distributed forces acting on beam
 *  [in]pfCount: number of distributed forces
 *
 * Return:
 *  bool: false on fail (right now its only when we cannot seperate sections)
*/
bool solveBeam(Beam * beam,
		PointForce pointForces[], int pfCount,
		DistributedForce distributedForces[], int dfCount)
{
	Section * rawSections = beam->raws;
	Section * shearSections = beam->shears;
	Section * momentSections = beam->moments;
    // Need to make sure the beam sections are cleared before we do calculations
    for (int i = 0; i < beam->sections_count; i++)
    {
        rawSections[i] = (Section){0};
        shearSections[i] = (Section){0};
        momentSections[i] = (Section){0};
    }
	float beamLength = beam->length;

    if (!seperateBeamIntoSections(
                beamLength, 
                pointForces, pfCount,
                distributedForces, dfCount, 
                rawSections, &beam->sections_count)
        )
	{
		// ERROR: forces result in too many sections (number of sections > allocated sections)
		return false;
	}
	int sectionsCount = beam->sections_count;
    beam->wall_reaction_force = calculateWallReactionForce(rawSections, sectionsCount);
    beam->wall_reaction_moment = calculateWallReactionMoment(rawSections, sectionsCount);
	solveShearSections(shearSections, rawSections, sectionsCount);
	solveMomentSections(momentSections, shearSections, rawSections, sectionsCount);
	return true;
}

#endif // SOMP_LOGIC_IMPLEMENTATION
#endif // SOMP_LOGIC_H
