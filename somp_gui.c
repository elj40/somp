/*
* Filename:	main.c
* Date:		03/06/2025
* Name:		EL Joubert
*
* Somp GUI
* The a frontend for the somp_logic code
*/

#define TODO(msg) printf("[%s, %d] TODO: "msg"\n",__FILE__, __LINE__);

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    int x;
    int xs;
} SompState;

void print_somp_state(SompState state) {
    printf("SompState {\n");
    printf("  r: %u\n", state.r);
    printf("  g: %u\n", state.g);
    printf("  b: %u\n", state.b);
    printf("  x: %d\n", state.x);
    printf("  xs: %d\n", state.xs);
    printf("}\n");
}
SompState * somp_state;

void somp_init(void * state)
{
    if (state) somp_state = (SompState *) state;
    else somp_state = malloc(sizeof(SompState));

    somp_state->r = 255;
    somp_state->g = 255;
    somp_state->b = 255;
    somp_state->xs = 1;
    somp_state->x = 0;
};

void somp_reload(void * state)
{
    if (state) somp_state = (SompState *) state;
    else somp_state = malloc(sizeof(SompState));

    somp_state->r = rand()%255;
    somp_state->g = rand()%255;
    somp_state->b = rand()%255;
};

void * somp_close()
{
    return somp_state;
};

// Main loop, returns false to exit application
bool somp_main(SDL_Renderer * sdl_renderer)
{
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type) {
            case SDL_EVENT_QUIT: return false;
        }
    }
    SDL_SetRenderDrawColor(sdl_renderer, somp_state->r, somp_state->g, somp_state->b, 255);
    SDL_RenderClear(sdl_renderer);

    SDL_FRect r = { somp_state->x, 100, 20, 20 };
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(sdl_renderer, &r);

    SDL_RenderPresent(sdl_renderer);

    somp_state->x += somp_state->xs;
    if (somp_state->x > 200 || somp_state->x < 0) somp_state->xs *= -1;

    SDL_Delay(3);

    return true;
}

