#ifndef SOMP_LOGIC_H
#define SOMP_LOGIC_H

#include <stdbool.h>
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
	int sectionsCount;
	Section raws[MAX_SECTIONS];
	Section shears[MAX_SECTIONS];
	Section moments[MAX_SECTIONS];
};
typedef struct Beam Beam;

void integratePolynomial(float dest[MAX_POLYNOMIAL_DEGREE], const float src[MAX_POLYNOMIAL_DEGREE]);
float evalPolynomial(float x, float poly[]);
int compSections(Section a, Section b);
void printSection(const void * vp);
void printPF(const void * vp);
void printDF(const void * vd);
void printDFptr(const void * vd);
void printStructArray(const void * arr, int count, int size, void (* printStruct)(const void *) );
void LL_SumDistributedPolynomials(LL_Node * head, float poly[]);
int compPointDists(const void * a, const void * b);
int compDistributedStartsPtr(const void * a, const void * b);
int compDistributedStarts(const void * a, const void * b);
int compDistributedEnds(const void * a, const void * b);
int compDistributedEndsPtr(const void * a, const void * b);
bool seperateBeamIntoSections(float beamLength,
		PointForce pForces[],       int pfCount, 
		DistributedForce dForces[], int dfCount, 
		Section sections[],         int * sectionsCount);
void printPolynomial(float p[MAX_POLYNOMIAL_DEGREE]);
float calculateWallReactionMoment(Section sections[], int sectionsCount);
float calculateWallReactionForce(Section sections[], int sectionsCount);
float evalPolynomial(float x, float poly[MAX_POLYNOMIAL_DEGREE]);
void integratePolynomial(float dest[MAX_POLYNOMIAL_DEGREE], const float src[MAX_POLYNOMIAL_DEGREE]);
void solveShearSections(Section shear[], Section raw[], int count);
void solveMomentSections(Section moment[], Section shear[], Section raw[], int count);
bool solveBeam(Beam * beam,
		PointForce pointForces[], int pfCount,
		DistributedForce distributedForces[], int dfCount);

#endif // SOMP_LOGIC_H
