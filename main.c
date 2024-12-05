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
* otherwise how do we represent a point force as a polynomial?
* Represent polynomials as arrays?
*/

#include <stdio.h>
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
void renderBeam(SDL_Renderer * pRend, SDL_Rect beam, float force, int resolution);

float calculateStress(float x, float x1, float force1, float x2, float force2, float x3, float force3);
float calculateStressDiscrete(float x, float distances[], float forces[], int forceCount);
float calculateStressXLinear(float x, float polyEnd);
float calculateStressXSquared(float x, float polyEnd);
float calculateStressPolynomial(float x, Polynomial poly);

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
	float distances[] = { 1, 2, 3, 4};

	for (int i = 0; i < 4; i++)
	{
		float stress = calculateStressXLinear(i, distances[3]);
		printf("V(%f) = %f\n", (float)i, stress);
	}

	Polynomial linear = {0};
	linear.start = 0;
	linear.end = 4;
	linear.coefficients[1] = 1;

	printf("\n");
	for (int i = 0; i < 4; i++)
	{
		float stress = calculateStressPolynomial( (float) i, linear);
		printf("V(%f) = %f\n", (float)i, stress);
	}

#if 0

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

#endif
	return 0;
}

void integratePolynomial(float integral[MAX_POLYNOMIAL_DEGREE], float poly[MAX_POLYNOMIAL_DEGREE])
{
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE-1; i++)
	{
		integral[i+1] = (1.0 / (i+1)) * poly[i];
		/* printf("%d: %f\n", i, (1.0 / (i+1)) * poly[i]); */
	}
}

float calculateStressXLinear(float x, float polyEnd)
{
	return 1.0 / 2.0 * (pow(polyEnd, 2) - pow(x,2));
}

float calculateStressXSquared(float x, float polyEnd)
{
	return 1.0 / 3.0 * (pow(polyEnd, 3) - pow(x,3));
}

void printPolynomial(float w[MAX_POLYNOMIAL_DEGREE])
{
	for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
	{
		if (w[i] != 0.0) 
		{
			printf("%.2fx^%d", w[i], i);
			if (i < MAX_POLYNOMIAL_DEGREE-1) printf(" + ");
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

float calculateStressPolynomial(float x, Polynomial poly)
{
	float V[MAX_POLYNOMIAL_DEGREE] = {0}; 
	integratePolynomial(V, poly.coefficients);

	printf("w:  ");
	printPolynomial(poly.coefficients);
	printf("V:  ");
	printPolynomial(V);

	// ---x---s---x----e-----x
	if (x > poly.end) return 0;
	else if (poly.start <= x && x <= poly.end)
	{
		float stress = 0;
		for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
		{
			stress += V[i]*pow(poly.end, i) - V[i]*pow(x, i);
		}
		return stress;
	} else
	{
		float stress = 0;
		for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
		{
			stress += V[i]*pow(poly.end, i) - V[i]*pow(poly.start, i);
		}
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

float calculateStress(float x, float x1, float force1, float x2, float force2, float x3, float force3)
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
	
	if ( 0 <= x && x <= x1)
	{
		return force1 + force2 + force3;
	} 
	else if ( x1 < x && x <= x2 )
	{
		return force2 + force3;
	}
	else if ( x2 < x && x <= x3 )
	{
		return force3;
	}

	return 0;
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

	static int f = 0;
	f += 10;

	renderBeam(pRend, beamRect, f, 30);

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

void renderBeam(SDL_Renderer * pRend, SDL_Rect beam, float force, int resolution)
{
	int stepWidth = beam.w / resolution;

	SDL_Rect rect;

	for (int i = 0; i < resolution; i++)
	{
		rect.x = beam.x + stepWidth*i;
		rect.y = beam.y;
		rect.w = stepWidth;
		rect.h = beam.h;

		float x1 = beam.w * 0.25, f1 = 5;
		float x2 = beam.w * 0.33, f2 = 2;
		float x3 = beam.w * 0.75, f3 = 2;
		float beamStress = calculateStress(stepWidth*i, x1, f1, x2, f2, x3, f3);
		int maxStress = 10;

		SDL_Color color = colorFromStress(beamStress, maxStress);

		int ax = beam.x + stepWidth * (i+1);
		int asy = beam.y - (beamStress/maxStress) * (windowHeight * 0.33);
		int aey = beam.y;
		renderArrow(pRend, WHITE, ax, asy, ax, aey, 10, 6);

		SDL_SetRenderDrawColor(pRend, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(pRend, &rect);
	}

	globalPauseUpdate = 1;

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
	float psi = atan2(dy, dx);
	float theta = atan2(b, l-a);

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
