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
*/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_POLYNOMIAL_DEGREE 10


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
	printf("Hello world!\n");

	/* float forces[] = { 1, 1, 1, 1 }; */
	/* float distances[] = { 1, 2, 3, 4}; */

	Polynomial squared = {0};
	squared.start = 0;
	squared.end = 4;
	squared.coefficients[2] = 1;

	/* Polynomial polynomials[2] = { squared, (Polynomial){0} }; */
	Polynomial polynomials[2] = { squared, squared };

	printf("\n");
	for (int i = 0; i < squared.end; i++)
	{
		float stress = calculateStressPolynomials( (float) i, polynomials, 2);
		printf("V(%f) = %f\n", (float)i, stress);
	}


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
			render(pWindow, pRenderer);
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
	// => Fwall - force1 - V2 = 0
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

void render(SDL_Window * pWin, SDL_Renderer * pRend)
{
	SDL_SetRenderDrawColor(pRend, 20, 20, 20, 255);
	SDL_RenderClear(pRend);

	//Draw wall
	SDL_SetRenderDrawColor(pRend, 255, 255, 255, 255);
	SDL_RenderDrawLine(pRend, (int) (0.25 * windowWidth), (int) (0.25 * windowHeight), (int) (0.25 * windowWidth), (int) (0.75 * windowHeight));

	// Draw beam
	int beamThickness = windowHeight * 0.1;
	int beamX = 0.25 * windowWidth;
	int beamY = 0.5 * windowHeight - beamThickness/2;
	int beamW = 0.5 * windowWidth;
	SDL_Rect beamRect = { beamX, beamY, beamW, beamThickness};

	renderBeam(pRend, beamRect, 20);

	SDL_SetRenderDrawColor(pRend, 255, 255, 255, 255);
	SDL_RenderDrawRect(pRend, &beamRect);


	SDL_RenderPresent(pRend);
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

void renderBeam(SDL_Renderer * pRend, SDL_Rect beam, int resolution)
{
	float forces[3] = { 1, 2, 5 };
	float distances[3] = { 100, 200, beam.w - 50 }; 

	int n = sizeof(forces)/sizeof(float);

	SDL_Rect rect;

#if 0
	for (int i = 0; i < n; i++)
	{
		rect.x = (n == 0) ? 0 : beam.x + distances[i-1];
		rect.y = beam.y;
		rect.h = beam.h;

		if (n == 0) rect.w = distances[0];
		else rect.w = distances[i]-distances[i-1];

		float beamStress = calculateStressDiscrete(distances[i], distances, forces, 3);
		int maxStress = 10;

		SDL_Color color = colorFromStress(beamStress, maxStress);

		int ax = rect.x + rect.w;
		int asy = beam.y - (forces[i]/5) * (beam.y - windowHeight / 4);
		int aey = beam.y;
		renderArrow(pRend, WHITE, ax, asy, ax, aey, 10, 6);

		SDL_SetRenderDrawColor(pRend, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(pRend, &rect);
	}

#endif

	//render linear poly
	Polynomial linear = { 0, beam.w };
	linear.coefficients[1] = 10.0/beam.w;

	Polynomial constant = { 100, beam.w };
	constant.coefficients[0] = 8;

	Polynomial polys[] = {constant, linear};

	float maxStress = calculateStressPolynomials(0.0, polys, 2);
	for (int x = 1; x <= beam.w; x++)
	{
		float beamStress = calculateStressPolynomials((float) x, polys, 2);

		SDL_Color color = colorFromStress(beamStress, maxStress);
		SDL_SetRenderDrawColor(pRend, color.r, color.g, color.b, color.a);
		SDL_RenderDrawLine(pRend, beam.x+x, beam.y, beam.x+x, beam.y+beam.h);
	}


	SDL_SetRenderDrawColor(pRend, 0, 0, 0, 255);
	SDL_RenderFillRect(pRend, &rect);

	globalPauseUpdate = 1; // stop after every render so we dont consume too much cpu

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
