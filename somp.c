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
#include "utils.h" // somp_logic.h depends on this so it should go first
#include "somp_logic.h"

#include "SDL.h"

#define WINDOW_SIZE_FACTOR 30
#define WINDOW_WIDTH (16*WINDOW_SIZE_FACTOR)
#define WINDOW_HEIGHT (9*WINDOW_SIZE_FACTOR)
#define WINDOW_X 0
#define WINDOW_Y 100



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
	SDL_Event event;
	while (!program_finished)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT: program_finished = true; break;
			case SDL_KEYDOWN: 
				       if (event.key.keysym.sym == SDLK_ESCAPE) {
					       program_finished = true; break;
				       }
			}
		}
	}

	printf("Killing properly\n");

	SDL_DestroyWindow(pWindow);
	SDL_DestroyRenderer(pRenderer);
	SDL_Quit();
	
	return 0;
}
