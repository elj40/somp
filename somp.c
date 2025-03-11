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

#include "SDL.h"

#define WINDOW_SIZE_FACTOR 40
#define WINDOW_WIDTH (16*WINDOW_SIZE_FACTOR)
#define WINDOW_HEIGHT (9*WINDOW_SIZE_FACTOR)
#define WINDOW_X 0
#define WINDOW_Y 100


struct DownArrow
{
	int x; 
	int y;
	int size;
};

typedef struct DownArrow DownArrow;
void renderDownArrow(SDL_Renderer* renderer, int x, int y, int size);

int main(int argc, char * argv[])
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		printf("Failed to init SDL: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Window * pWindow = SDL_CreateWindow("SOMP",
			WINDOW_X, WINDOW_Y,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_RESIZABLE);

	if(!pWindow)
	{
		printf("Failed to init SDL_Window: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	
	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!pRenderer)
	{
		printf("Failed to init SDL_Renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(pWindow);
		SDL_Quit();
		return 1;
	}
	printf("Hello SOMP\n");

	bool program_finished = false;
	bool mouse_pressed = false;

	SDL_Event event;
	int mouseX, mouseY;
	Uint32 mouseState;

	Beam beam = {0};
	beam.length = 1.0;
	beam.sectionsCount = MAX_SECTIONS;

	PointForce pointForces[10]; //TODO: figure out how many we should allocate for
	int pfCount;
	pointForces[0] = (PointForce){ 0.0, 1 };
	pointForces[1] = (PointForce){ 0.25,2 };
	pointForces[2] = (PointForce){ 0.5, 3 };
	pointForces[3] = (PointForce){ 1.0, 4 };
	pfCount = 4;

	DistributedForce distributedForces[10];
	int dfCount;
	distributedForces[0] = (DistributedForce){ 0, 0.5, {1,0} };
	distributedForces[1] = (DistributedForce){ 0.25, 0.75, {2,0} };
	distributedForces[2] = (DistributedForce){ 0.75, 1.0, {3,0} };
	distributedForces[3] = (DistributedForce){ 0.65, 0.95, {4,0} };
	dfCount = 4;
	solveBeam(&beam, pointForces, pfCount, distributedForces, dfCount);


	// TODO: make a dynamic array utility
	while (!program_finished)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT: program_finished = true; break;
			case SDL_KEYDOWN: 
				       switch (event.key.keysym.sym) {
                       case SDLK_ESCAPE:
                           program_finished = true; break;
				       }
			}
		}

		mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        if (SDL_BUTTON(mouseState) == 1) printf("Clicked\n");

		SDL_SetRenderDrawColor(pRenderer, 0,0,0, 255);
		SDL_RenderClear(pRenderer);
		SDL_SetRenderDrawColor(pRenderer, 255,255,255, 255);
		
		for (int i = 0; i < pfCount; i++)
		{
			int x = (pointForces[i].distance/beam.length) * WINDOW_WIDTH;
			int y = WINDOW_HEIGHT - (pointForces[i].force/10) * WINDOW_HEIGHT;
			int size = WINDOW_HEIGHT - y;
			renderDownArrow(pRenderer, x, y, size);
		};

		SDL_RenderPresent(pRenderer);

		mouse_pressed = mouseState & SDL_BUTTON_LEFT; 

	}

	printf("Killing properly\n");

	SDL_DestroyWindow(pWindow);
	SDL_DestroyRenderer(pRenderer);
	SDL_Quit();
	
	return 0;
}

// Function to render a down-pointing arrow
// Written by ChatGPT
// TODO: write a better alternative than chatgpt
void renderDownArrow(SDL_Renderer* renderer, int x, int y, int size) {
    // Shaft length is proportional to the size
    int shaftLength = size * 2 / 3; 
    int arrowheadHeight = size - shaftLength;  // Remaining height for the arrowhead
    
    // Arrow shaft (vertical line)
    SDL_RenderDrawLine(renderer, x, y, x, y + shaftLength);

    // Arrowhead (triangle)
    SDL_RenderDrawLine(renderer, x - size / 8, y + shaftLength, x + size / 8, y + shaftLength);  // Base of the triangle
    SDL_RenderDrawLine(renderer, x - size / 8, y + shaftLength, x, y + shaftLength + arrowheadHeight);  // Left diagonal
    SDL_RenderDrawLine(renderer, x + size / 8, y + shaftLength, x, y + shaftLength + arrowheadHeight);  // Right diagonal
}
