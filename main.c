/*
* Filename:	main.c
* Date:		22/11/2024 
* Name:		EL Joubert
*
* Main file of somp (Strength of Materials Project) which will attempt to
* display the effects of different loads on a beam
*
* For now we will only consider single point forces
* We wont support overalapping forces for now either
*
* It seems that we have to consider point forces and functions seperately,
* otherwise how do we represent a point force as a polynomial/function?
* Represent polynomials as arrays?
*
* 14/12/2024: try and render all the graphs and arrows and stuff
* TODO: learn how to hot reload
*/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "utils.c" //TODO, make sure this c file doesnt cause issues

#define MAX_POLYNOMIAL_DEGREE 10

struct Beam {
	float length;
	SDL_Rect screenRect;
};

typedef struct Beam Beam;

struct Polynomial {
	float start;
	float end;
 	// Position in array determines power of x
	// => [3,2,4,0,1] = 3 + 2x + 4x^2 + x^4
	float coefficients[MAX_POLYNOMIAL_DEGREE];
};

typedef struct Polynomial Polynomial;

void printPolynomial(float w[MAX_POLYNOMIAL_DEGREE]);
void printFullPolynomial(float w[MAX_POLYNOMIAL_DEGREE]);

void handleEvents(SDL_Window * pWin, SDL_Renderer * pRend);

void render(SDL_Window * pWin, SDL_Renderer * pRend);
void renderTriangle(SDL_Renderer * pRend, SDL_Color color, int x1, int y1,int x2, int y2,int x3, int y3);
void renderArrow(SDL_Renderer * pRend, SDL_Color color, int startX, int startY, int endX, int endY, int tipLength, int tipWidth);

float calculateStress(float x, float x1, float force1, float x2, float force2, float x3, float force3);
float calculateStressDiscrete(float x, float distances[], float forces[], int forceCount);
float calculateStressPolynomial(float x, Polynomial poly);
float calculateStressPolynomials(float x, Polynomial polys[], int polyCount);

SDL_Color colorFromStress(float stress, float maxStress);

#define TARGET_FRAME_TIME_MS (1000/60)
#define WINDOW_FACTOR 50
int windowWidth = 16*WINDOW_FACTOR;
int windowHeight = 9*WINDOW_FACTOR;

#define WHITE (SDL_Color){255,255,255,255}

int globalCloseWindow = 0;
int globalPauseUpdate = 0;

int main(int argc, char * argv[]) 
{
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window * pWindow = SDL_CreateWindow("SOMP", 0, 200, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
	
	Uint64 startTime, endTime;
	float elapsedTime = (float) SDL_GetTicks();
	while (!globalCloseWindow)
	{
		startTime = SDL_GetPerformanceCounter();
		if (TARGET_FRAME_TIME_MS - elapsedTime > 0)
		{
			SDL_Delay(TARGET_FRAME_TIME_MS - elapsedTime);
		}

		handleEvents(pWindow, pRenderer);

		if (!globalPauseUpdate)
		{
			SDL_SetRenderDrawColor(pRenderer, 20, 20, 20, 255);
			SDL_RenderClear(pRenderer);

			SDL_Rect target = {0, 0, windowWidth, windowHeight};

			renderBeamArea(pRenderer, target);

			SDL_RenderPresent(pRenderer);
		}

		endTime = SDL_GetPerformanceCounter();
		elapsedTime = (endTime - startTime) / (float) SDL_GetPerformanceFrequency();
	}

	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);

	SDL_Quit();

	return 0;
}

void integratePolynomial(Polynomial * integral, Polynomial * poly)
{
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE-1; i++)
	{
		integral->coefficients[i+1] = (1.0 / (i+1)) * poly->coefficients[i];
	}
}


void printPolynomial(float w[MAX_POLYNOMIAL_DEGREE])
{
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		if (w[i] != 0.0) 
		{
			if (i != 0) printf(" + ");
			printf("%.2fx^%d", w[i], i);
		}
	}
	printf("\n");
}

void printFullPolynomial(float w[MAX_POLYNOMIAL_DEGREE])
{
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		printf("%.2fx^%d", w[i], i);
		if (i < MAX_POLYNOMIAL_DEGREE-1) printf(" + ");
	}
	printf("\n");
}

float calculateStressPolynomials(float x, Polynomial polys[], int polyCount)
{
	float totalStress = 0;
	for (int i = 0; i < polyCount; i++)
	{
		totalStress += calculateStressPolynomial(x, polys[i]);
	}

	return totalStress;
};

float calculatePolynomial(float x, Polynomial p)
{
		float value = 0;
		for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
		{
			value += p.coefficients[i]*pow(x, i);
		}
		return value;
}

float calculateStressPolynomial(float x, Polynomial poly)
{

	Polynomial integral = { poly.start, poly.end };
	integratePolynomial(&integral, &poly);

	// Force could be before, inside or after range
	// ---x---s---x----e-----x
	if (x > poly.end) return 0;
	else if (poly.start <= x && x <= poly.end)
	{
		float stress = calculatePolynomial(poly.end, integral) - calculatePolynomial(x, integral);
		return stress;
	} else
	{
		float stress = calculatePolynomial(poly.end, integral) - calculatePolynomial(poly.start, integral);
		return stress;
	}
	
}

float calculateStressDiscrete(float x, float distances[], float forces[], int forceCount)
{
	// Global sum of Fy = 0
	// => Fwall = force1 + force2 + force3
	//
	// Section1 sum Fy = 0
	// => Fwall - V1 = 0
	// => v1 = force1 + force2 + force3
	//
	// Section2 sum Fy = 0
	// => Fwall - force1 - V2 = /
	// => v2 = force2 + force3
	//
	// Section3 sum Fy = 0
	// => Fwall - force1 - force2 - v3 = 0
	// => v3 = force3
	
	// Can store runnning sum in an array, or maybe only need single variable
	int i = forceCount - 1;
	float stress = 0;

	while (x <= distances[i] && i >= 0) 
	{
		stress += forces[i--];
	}

	return stress;
}

void handleEvents(SDL_Window * pWin, SDL_Renderer * pRend)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			globalCloseWindow = 1;
			return;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_RETURN) globalCloseWindow = 1; // Quit on enter
			else if (event.key.keysym.sym == SDLK_SPACE) globalPauseUpdate = 0; //Unpause on space
			break;
		case SDL_WINDOWEVENT:
			SDL_GetWindowSize(pWin, &windowWidth, &windowHeight);
			break;
		default:
			break;
		}
	}
}

SDL_Color colorFromStress(float stress, float maxStress)
{
	int value = (float)stress / maxStress * 255;

	if (stress > maxStress)
		value = 255;

	SDL_Color c = { value, value, value, 255 };
	return c;
}


void renderBeamStress(SDL_Renderer * pRend, Beam beam, float forces[], float distances[], int forceCount, Polynomial polys[], int polyCount)
{

	// TODO: calculate everything in a seperate function then render it here (really? what advantages does this give us?)
	float maxStress = calculateStressPolynomials(0.0, polys, polyCount);
	for (int x = 1; x <= beam.screenRect.w; x++)
	{
		float beamStress = calculateStressPolynomials((float) x, polys, polyCount);

		SDL_Color color = colorFromStress(beamStress, maxStress);
		SDL_SetRenderDrawColor(pRend, color.r, color.g, color.b, color.a);
		SDL_RenderDrawLine(pRend, beam.screenRect.x+x, beam.screenRect.y, beam.screenRect.x+x, beam.screenRect.y+beam.screenRect.h);
	}

	globalPauseUpdate = 1; // stop after every render so we dont consume too much cpu

}

#define WALL_PADDING_X 0.25 // percentage of target rect
#define WALL_PADDING_TOP 0.25 // percentage of target rect
#define WALL_PADDING_BOTTOM 0.75 // percentage of target rect

#define BEAM_THICKNESS_PERCENT 0.1 // percent of target rect
#define BEAM_WIDTH_PERCENT 0.5 // percent of target rect

void renderBeamArrows(SDL_Renderer * pRend, Beam beam, float forces[], float distances[], int forceCount, Polynomial polys, int polyCount)
{
	
	/* float maxForce = ArrayMaxf(forces, forceCount); */
	/* for (int i = 0; i < forceCount; i++) */
	/* { */
	/* 	int arrowStartX = 1; */
	/* } */
}

void renderBeamArea(SDL_Renderer * pRend, SDL_Rect target)
{
	// render wall line
	SDL_SetRenderDrawColor(pRend, 255, 255, 255, 255);
	SDL_RenderDrawLine(pRend, 
			(int) (WALL_PADDING_X * target.w + target.x),
			(int) (WALL_PADDING_TOP * target.h + target.y),
			(int) (WALL_PADDING_X * target.w + target.x),
			(int) (WALL_PADDING_BOTTOM * target.h + target.y));

	int beamThickness = target.y + target.h * BEAM_THICKNESS_PERCENT;
	int beamX = WALL_PADDING_X * target.w + target.x;
	int beamY = 0.5 * target.h + target.y - beamThickness/2;
	int beamW = BEAM_WIDTH_PERCENT * target.w + target.x;
	SDL_Rect beamRect = { beamX, beamY, beamW, beamThickness};

	float forces[1];
	float distances[1];
	Polynomial polys[1];

	Beam beam;
	beam.length = 1.0;
	beam.screenRect = beamRect;

	// render beam stress
	renderBeamStress(pRend, beam, forces, distances, 0, polys, 0);
	
	// render beam outline
	SDL_SetRenderDrawColor(pRend, 255, 255, 255, 255);
	SDL_RenderDrawRect(pRend, &beamRect);

	// render beam arrows
	/* renderBeamArrows(pRend, target, forces, distances, 2, polynomials, 3); */

	// render graphs for each polynomial
	/* renderBeamGraphs(pRend, target, polynomials, 3); */
}

void renderTriangle(SDL_Renderer * pRend, SDL_Color color, int x1, int y1,int x2, int y2,int x3, int y3) 
{
	SDL_FPoint p1 = {x1,y1};
	SDL_FPoint p2 = {x2,y2};
	SDL_FPoint p3 = {x3,y3};

	SDL_Vertex vertices[3];
	vertices[0].position = p1;
	vertices[1].position = p2;
	vertices[2].position = p3;

	for (int i = 0; i < 3; ++i)
	{
		vertices[i].color = color;
	}

	SDL_RenderGeometry(pRend, NULL, vertices, 3, NULL, 0);

}

void renderArrow(SDL_Renderer * pRend, SDL_Color color, int startX, int startY, int endX, int endY, int tipLength, int tipWidth)
{
	float dx = endX - startX;
	float dy = endY - startY;

	float arrow_length = sqrt(dx*dx + dy*dy);

	float a = tipLength;
	float b = tipWidth/2;
	float l = arrow_length;

	float d = sqrt((l-a)*(l-a)+b*b);
	double psi = atan2(dy, dx);
	double theta = atan2(b, l-a);

	float px1 = d*cos(psi-theta);
	float py1 = d*sin(psi-theta);

	float px2 = d*cos(psi+theta);
	float py2 = d*sin(psi+theta);

	SDL_SetRenderDrawColor(pRend, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(pRend, startX, startY, endX, endY);
	renderTriangle(pRend, color,
			endX, endY, 
			px1 + startX, py1 + startY, 
			px2 + startX, py2 + startY
			);
}
