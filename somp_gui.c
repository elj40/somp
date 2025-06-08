/*
* Filename:	main.c
* Date:		03/06/2025
* Name:		EL Joubert
*
* Somp GUI
* The a frontend for the somp_logic code
* Styling for now is inspired by:
* R.C. Hibbeler's "Mechanics of Materials 10th ed."
*/

#define TODO(msg) printf("[%s, %d] TODO: "msg"\n",__FILE__, __LINE__);

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

typedef enum {
    NORMAL,
    ADD_POINT_FORCE,
    ADD_DISTRIBUTED,
} Mode;

typedef struct {
    Beam beam;
    PointForces point_forces;
    DistributedForces distrib_forces;
    Mode mode;
} somp_section_solve_t;

typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
    struct {
        somp_section_solve_t solve;
    } section;
} SompState;

SompState * somp_state;
int sdl_window_width, sdl_window_height;

void somp_init(void * state)
{
    if (state) somp_state = (SompState *) state;
    else somp_state = malloc(sizeof(SompState));

    Beam * b = &somp_state->section.solve.beam;
    PointForces * pfs = &somp_state->section.solve.point_forces;
    DistributedForces * dfs = &somp_state->section.solve.distrib_forces;

    *pfs = (PointForces){0};
    *dfs = (DistributedForces){0};

    b->length = 1.0;
    b->sections_count = MAX_SECTIONS;

    DynamicArrayAppend(dfs , ((DistributedForce){ .start=0.0, .end=0.33, .polynomial={ 0, 1 } }));
};

void somp_reload(void * state, SDL_Window * sdl_window, SDL_Renderer * sdl_renderer)
{
    (void)sdl_renderer;
    if (state) somp_state = (SompState *) state;
    else somp_state = malloc(sizeof(SompState));

    SDL_GetWindowSize(sdl_window, &sdl_window_width, &sdl_window_height);
    printf("Reload\n");
};

void * somp_close()
{
    return somp_state;
};

#define EJSDL_ARROWH_L 10
#define EJSDL_ARROWH_W 5

void ejsdl_render_arrow_vert(SDL_Renderer * sdl_renderer,
        float x, float y1, float y2)
{
    float arrow_head_length = (y2 < y1) ? -EJSDL_ARROWH_L : EJSDL_ARROWH_L;

    SDL_RenderLine(sdl_renderer, x, y1, x, y2);
    SDL_RenderLine(sdl_renderer, x + EJSDL_ARROWH_W, y2 - arrow_head_length, x, y2);
    SDL_RenderLine(sdl_renderer, x - EJSDL_ARROWH_W, y2 - arrow_head_length, x, y2);
}

#define COLOR_BLACK             0,   0,   0, 255
#define COLOR_BLACK_CLEAR       0,   0,   0, 170
#define COLOR_HIBB_BACKGROUND 255, 252, 233, 255
#define COLOR_HIBB_BEAM        79, 167, 195, 255

#define DISTRIB_STEP 4

// Main loop, returns false to exit application
bool somp_main(SDL_Window * sdl_window, SDL_Renderer * sdl_renderer)
{

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type) {
            case SDL_EVENT_QUIT: return false;
            case SDL_EVENT_WINDOW_SHOWN:
            case SDL_EVENT_WINDOW_RESIZED:
                 SDL_GetWindowSize(sdl_window, &sdl_window_width, &sdl_window_height);
                 break;
            case SDL_EVENT_KEY_DOWN:
                 switch (sdl_event.key.key) {
                 case SDLK_S: {
                     somp_section_solve_t * s = &somp_state->section.solve;
                     solveBeam(&s->beam,
                             s->point_forces.items, s->point_forces.count,
                             s->distrib_forces.items, s->distrib_forces.count);
                    printf("Beam: {\n");
                    printf("\t.length = %f\n", s->beam.length);
                    printf("\t.sections_count = %d\n", s->beam.sections_count);
                    printf("Raw: ");
                    printStructArray(s->beam.raws, s->beam.sections_count, sizeof(Section), printSection);
                    //printf("shear: ");
                    //printStructArray(s->beam.shears, s->beam.sections_count, sizeof(Section), printSection);
                    //printf("moment: ");
                    //printStructArray(s->beam.moments, s->beam.sections_count, sizeof(Section), printSection);
                    printf("}\n");
                 }; break;
                 case SDLK_F: {
                     somp_section_solve_t * s = &somp_state->section.solve;
                     s->mode = (s->mode == ADD_POINT_FORCE) ? NORMAL : ADD_POINT_FORCE;
                 }; break;
                 }
        }
    }
    // ========================= SOLVE SECTION START ========================
    somp_section_solve_t * state = &somp_state->section.solve;
    SDL_SetRenderDrawColor(sdl_renderer, COLOR_HIBB_BACKGROUND);
    SDL_RenderClear(sdl_renderer);

    SDL_FRect beam_rect = { 0.1*sdl_window_width, 0.5*sdl_window_height, 0.5*sdl_window_width, 10 };
    SDL_SetRenderDrawColor(sdl_renderer, COLOR_HIBB_BEAM);
    SDL_RenderFillRect(sdl_renderer, &beam_rect);
    SDL_SetRenderDrawColor(sdl_renderer, COLOR_BLACK);
    SDL_RenderRect(sdl_renderer, &beam_rect);

    SDL_SetRenderDrawColor(sdl_renderer, COLOR_BLACK);
    SDL_RenderLine(sdl_renderer, beam_rect.x, 0.1*sdl_window_height, beam_rect.x, 0.9*sdl_window_height);

    for (int i = 0; i < state->point_forces.count; i++)
    {
        PointForce pf = state->point_forces.items[i];
        int x = beam_rect.x + (pf.distance/state->beam.length)*(beam_rect.w);
        ejsdl_render_arrow_vert(sdl_renderer, x, 0.1*sdl_window_height, beam_rect.y);
    }

    for (int i = 0; i < state->distrib_forces.count; i++)
    {
        DistributedForce df = state->distrib_forces.items[i]; 
        int x_start = beam_rect.x + (df.start/state->beam.length)*(beam_rect.w);
        int x_end   = beam_rect.x + (df.end /state->beam.length)*(beam_rect.w);
        for (int x = x_start; x <= x_end; x += DISTRIB_STEP)
        {
            int y = beam_rect.y - evalPolynomial(x-beam_rect.x, df.polynomial)*0.1;
            SDL_RenderLine(sdl_renderer, x, y, x, beam_rect.y);
        };
    }

    switch (state->mode) {
    case ADD_POINT_FORCE: {
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);

        SDL_SetRenderDrawColor(sdl_renderer, COLOR_BLACK_CLEAR);
        int x = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
        ejsdl_render_arrow_vert(sdl_renderer, x, 0.1*sdl_window_height, beam_rect.y);

        if (mouse_button & SDL_BUTTON_LEFT)
        {
            float d = ((x-beam_rect.x)/beam_rect.w)*state->beam.length;
            float f = 1.0;
            PointForce p = { .distance=d, .force=f };
            DynamicArrayAppend(&state->point_forces, p);
            state->mode = NORMAL;
            printf("Adding pointforce\n");
        }
    default: break;
    }
    }

    // ========================= SOLVE SECTION END ========================

    SDL_RenderPresent(sdl_renderer);

    SDL_Delay(3);

    return true;
}

