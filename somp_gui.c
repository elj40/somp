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
    MOD_POINT_FORCE,
    MOD_DISTRIBUTED_START,
    MOD_DISTRIBUTED_END  ,
} Mode;

typedef struct {
    Beam beam;
    PointForces point_forces;
    DistributedForces distrib_forces;
    Mode mode;
    // For previewing the distributed load to be added
    bool distributed_first_placed;
    // keeps start and end points
    SDL_FRect distrib_prev_se;

    // Pointers to keep track of the current point or distributed force being
    // modified
    union {
        PointForce * mod_point;
        DistributedForce * mod_distrib;
    };
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

    somp_state->section.solve.distributed_first_placed = false;

    Beam * b = &somp_state->section.solve.beam;
    PointForces * pfs = &somp_state->section.solve.point_forces;
    DistributedForces * dfs = &somp_state->section.solve.distrib_forces;

    *pfs = (PointForces){0};
    *dfs = (DistributedForces){0};

    b->length = 1.0;
    b->sections_count = MAX_SECTIONS;
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

float ejsdl_distrib_line_y(
        float x,
        SDL_FRect x_axis,
        float max_value,
        float polynomial[]
        )
{
        float bx = lerp(0, max_value, (x-x_axis.x)/x_axis.w);
        float y = x_axis.y - evalPolynomial(bx, polynomial);
        return y;
};

bool mouse_pressed = false;
bool mouse_released = false;

#define COLOR_DEFAULT         COLOR_BLACK
#define COLOR_RED             255,   0,   0, 255
#define COLOR_BLACK             0,   0,   0, 255
#define COLOR_GRAY            100, 100, 100, 255
#define COLOR_HIBB_BACKGROUND 255, 252, 233, 255
#define COLOR_HIBB_BEAM        79, 167, 195, 255

void somp_solve_normal(SDL_Renderer * sdl_renderer, SDL_FRect beam_rect)
{
#define HIGHLIGHT_DISTANCE 5
    const bool * keyboard_state = SDL_GetKeyboardState(NULL);
    somp_section_solve_t * S = &somp_state->section.solve;
    float mouse_x, mouse_y;
    int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);

    SDL_SetRenderDrawColor(sdl_renderer, COLOR_RED);
    for (int i = 0; i < S->point_forces.count; i++)
    {
        PointForce pf = S->point_forces.items[i];
        float fx = lerp(beam_rect.x, beam_rect.x+beam_rect.w, pf.distance/S->beam.length);
        if (fabs(fx - mouse_x) < HIGHLIGHT_DISTANCE)
        {
            ejsdl_render_arrow_vert(sdl_renderer, fx, 0.1*sdl_window_height, beam_rect.y);
            if (mouse_button & SDL_BUTTON_LEFT && mouse_pressed)
            {
                S->mode = MOD_POINT_FORCE;
                S->mod_point = &S->point_forces.items[i];
            } else if (keyboard_state[SDL_SCANCODE_X])
            {
                shift_array(S->point_forces.items, i, S->point_forces.count)
                S->point_forces.count--;
                return;
            };
        };
    };

    for (int i = 0; i < S->distrib_forces.count; i++)
    {
        // Render distributed force
        DistributedForce df = S->distrib_forces.items[i];
        int x_start = lerp(beam_rect.x, beam_rect.x+beam_rect.w, df.start/S->beam.length);
        int x_end   = lerp(beam_rect.x, beam_rect.x+beam_rect.w, df.end  /S->beam.length);

        if (fabs(x_start - mouse_x) < HIGHLIGHT_DISTANCE)
        {
            float y = ejsdl_distrib_line_y(x_start, beam_rect, S->beam.length, df.polynomial);
            SDL_RenderLine(sdl_renderer, x_start, y, x_start, beam_rect.y);
            if (mouse_button & SDL_BUTTON_LEFT && mouse_pressed)
            {
                S->mode = MOD_DISTRIBUTED_START;
                S->mod_distrib = &S->distrib_forces.items[i];
            }else if (keyboard_state[SDL_SCANCODE_X])
            {
                // shift elements after this up by one
                shift_array(S->distrib_forces.items, i, S->distrib_forces.count)
                // decrease count by one
                S->distrib_forces.count--;
            };

        } else if (fabs(x_end - mouse_x) < HIGHLIGHT_DISTANCE)
        {
            float y = ejsdl_distrib_line_y(x_end, beam_rect, S->beam.length, df.polynomial);
            SDL_RenderLine(sdl_renderer, x_end, y, x_end, beam_rect.y);
            if (mouse_button & SDL_BUTTON_LEFT && mouse_pressed)
            {
                S->mode = MOD_DISTRIBUTED_END;
                S->mod_distrib = &S->distrib_forces.items[i];
            }else if (keyboard_state[SDL_SCANCODE_X])
            {
                // shift elements after this up by one
                shift_array(S->distrib_forces.items, i, S->distrib_forces.count)
                // decrease count by one
                S->distrib_forces.count--;
            };

        }
    };
    SDL_SetRenderDrawColor(sdl_renderer, COLOR_DEFAULT);
}

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
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mouse_released = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mouse_pressed = true;
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
                    printf("shear: ");
                    printStructArray(s->beam.shears, s->beam.sections_count, sizeof(Section), printSection);
                    printf("moment: ");
                    printStructArray(s->beam.moments, s->beam.sections_count, sizeof(Section), printSection);
                    printf("}\n");
                 }; break;
                 case SDLK_F: {
                     somp_section_solve_t * s = &somp_state->section.solve;
                     s->mode = (s->mode == ADD_POINT_FORCE) ? NORMAL : ADD_POINT_FORCE;
                 }; break;
                 case SDLK_D: {
                     somp_section_solve_t * s = &somp_state->section.solve;
                     s->mode = (s->mode == ADD_DISTRIBUTED) ? NORMAL : ADD_DISTRIBUTED;
                     s->distributed_first_placed = false;
                 }; break;
                 case SDLK_R: {
                     somp_section_solve_t * s = &somp_state->section.solve;
                     s->point_forces.count = 0;
                     s->distrib_forces.count = 0;
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
// to ensure distributed lines are aligned
#define DISTRIB_VIEW_STEP 18
    for (int i = 0; i < state->distrib_forces.count; i++)
    {
        // Render distributed force
        DistributedForce df = state->distrib_forces.items[i];
        int x_start = lerp(beam_rect.x, beam_rect.x+beam_rect.w, df.start/state->beam.length);
        int x_end   = lerp(beam_rect.x, beam_rect.x+beam_rect.w, df.end  /state->beam.length);

        float y = ejsdl_distrib_line_y(x_start, beam_rect, state->beam.length, df.polynomial);

        SDL_RenderLine(sdl_renderer, x_start, y, x_start, beam_rect.y);

        int xp = x_start;
        float yp = y;
        // Align all lines (I think its neater)
        x_start +=  DISTRIB_VIEW_STEP - x_start % DISTRIB_VIEW_STEP;
        for (int x = x_start; x <= x_end; x += DISTRIB_VIEW_STEP)
        {
            y = ejsdl_distrib_line_y(x, beam_rect, state->beam.length, df.polynomial);
            // Line to beam
            SDL_RenderLine(sdl_renderer, x, y, x, beam_rect.y);
            // Line from previous
            SDL_RenderLine(sdl_renderer, xp, yp, x, y);

            xp = x;
            yp = y;
        };
        y = ejsdl_distrib_line_y(x_end, beam_rect, state->beam.length, df.polynomial);
        // Line to beam
        SDL_RenderLine(sdl_renderer, x_end, y, x_end, beam_rect.y);
        // Line from previous
        SDL_RenderLine(sdl_renderer, xp, yp, x_end, y);
    }

    switch (state->mode) {
    case NORMAL: {
        somp_solve_normal(sdl_renderer, beam_rect);
    }; break;
    case ADD_POINT_FORCE: {
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);

        SDL_SetRenderDrawColor(sdl_renderer, COLOR_GRAY);
        int x = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
        ejsdl_render_arrow_vert(sdl_renderer, x, 0.1*sdl_window_height, beam_rect.y);

        if (mouse_button & SDL_BUTTON_LEFT && mouse_pressed)
        {
            float d = ((x-beam_rect.x)/beam_rect.w)*state->beam.length;
            float f = 1.0;
            PointForce p = { .distance=d, .force=f };
            DynamicArrayAppend(&state->point_forces, p);
            printf("Adding pointforce\n");
        }
    }; break;
    case ADD_DISTRIBUTED: {
#define DISTRIB_PREVIEW_LOOKAHEAD (beam_rect.w/10)
#define DISTRIB_PREVIEW_HEIGHT 5
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
        float * dx_start = &state->distrib_prev_se.x;
        float * dx_end = &state->distrib_prev_se.y;
        float * dy_start = &state->distrib_prev_se.w;
        float * dy_end = &state->distrib_prev_se.h;

        // Distributed distances and heights, in pixels

        if (!state->distributed_first_placed)
        {
            SDL_SetRenderDrawColor(sdl_renderer, COLOR_GRAY);
            int x_start = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
            int x_end   = MIN(beam_rect.x + beam_rect.w, mouse_x + DISTRIB_PREVIEW_LOOKAHEAD);
            for (int x = x_start; x <= x_end; x += DISTRIB_VIEW_STEP)
            {
                int y = mouse_y;
                SDL_RenderLine(sdl_renderer, x, y, x, beam_rect.y);
            };
            if (mouse_button & SDL_BUTTON_LEFT && mouse_pressed)
            {
                *dx_start = x_start; //beam_rect.x + ((x_start-beam_rect.x)/beam_rect.w)*beam_rect.w;
                *dy_start = mouse_y;
                state->distributed_first_placed = true;
            }
        } else
        {
            SDL_SetRenderDrawColor(sdl_renderer, COLOR_GRAY);
            int x_start = *dx_start;
            int x_end = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
            int y_start = *dy_start;
            int y_end = mouse_y;
            if (x_end < x_start) {
                int t = x_end; x_end = x_start; x_start = t;
                    t = y_end; y_end = y_start; y_start = t;
            };
            // Render distributed preview
            for (float x = x_start; x <= x_end; x += DISTRIB_VIEW_STEP)
            {
                float t = (x-x_start)/(x_end-x_start);
                int y = y_start + t*(y_end - y_start);
                SDL_RenderLine(sdl_renderer, x, y, x, beam_rect.y);
            }
            if (mouse_released)
            {
                *dx_end = x_end;//beam_rect.x + ((x_start-beam_rect.x)/beam_rect.w)*state->beam.length;
                *dy_end = y_end;//beam_rect.y - mouse_y;
                float fs_x = lerp(0, state->beam.length, (x_start-beam_rect.x)/beam_rect.w);
                float fs_y = beam_rect.y - y_start; //TODO: normalize for forces
                float fe_x = lerp(0, state->beam.length, (x_end-beam_rect.x)/beam_rect.w);
                float fe_y = beam_rect.y - y_end; //TODO: normalize for forces

                printf("%f %f\n", fs_y, fe_y);

                // y = mx + c
                float m, c;
                line_from_points(&m, &c, fs_x, fs_y, fe_x, fe_y);

                DynamicArrayAppend(&state->distrib_forces,
                        ((DistributedForce){ .start=fs_x, .end=fe_x, .polynomial={c,m}})
                        );

                printStructArray(state->distrib_forces.items, state->distrib_forces.count, sizeof(DistributedForce), printDF);

                state->mode = NORMAL;
            }
        };
    }; break;
    case MOD_POINT_FORCE: {
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
        (void)mouse_button;

        SDL_SetRenderDrawColor(sdl_renderer, COLOR_RED);
        int x = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
        ejsdl_render_arrow_vert(sdl_renderer, x, 0.1*sdl_window_height, beam_rect.y);

        float d = ((x-beam_rect.x)/beam_rect.w)*state->beam.length;
        float f = 1.0;
        state->mod_point->distance = d;
        state->mod_point->force = f;
        if (mouse_released)
        {
            state->mode = NORMAL;
        }
        SDL_SetRenderDrawColor(sdl_renderer, COLOR_DEFAULT);
    }; break;
    case MOD_DISTRIBUTED_START: {
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
        (void)mouse_button;

        SDL_SetRenderDrawColor(sdl_renderer, COLOR_RED);

        int x_start = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
        int y_start = mouse_y;
        SDL_RenderLine(sdl_renderer, x_start, y_start, x_start, beam_rect.y);

        float m;
        float c;
        float fs_x = lerp(0, state->beam.length, (x_start-beam_rect.x)/beam_rect.w);
        float fs_y = beam_rect.y - y_start;
        float fe_x = state->mod_distrib->end;
        float fe_y = evalPolynomial(fe_x, state->mod_distrib->polynomial);

        if (fs_x >= fe_x)
        {
            state->mod_distrib->start = state->mod_distrib->end;
            state->mode = MOD_DISTRIBUTED_END;
            printf("Switch to MOD_DISTRIBUTED_END\n");
            break;
        }

        // We don't want funky things with divide by infinties;
        if (nearly_equal(fs_x, fe_x)) break;
        // Reset polynomial
        for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
        {
            state->mod_distrib->polynomial[i] = 0;
        }

        line_from_points(&m, &c, fs_x, fs_y, fe_x, fe_y);
        state->mod_distrib->polynomial[0] = c;
        state->mod_distrib->polynomial[1] = m;

        state->mod_distrib->start = fs_x;

        if (mouse_released)
        {
            state->mode = NORMAL;
        }
    }; break;
    case MOD_DISTRIBUTED_END: {
        float mouse_x, mouse_y;
        int mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
        (void)mouse_button;

        SDL_SetRenderDrawColor(sdl_renderer, COLOR_RED);
        int x_end = MAX(beam_rect.x, MIN(mouse_x, beam_rect.x + beam_rect.w));
        int y_end = mouse_y;
        SDL_RenderLine(sdl_renderer, x_end, y_end, x_end, beam_rect.y);

        float m = 0;
        float c = 0;

        float fe_x = lerp(0, state->beam.length, (x_end-beam_rect.x)/beam_rect.w);
        float fe_y = beam_rect.y - y_end;

        float fs_x = state->mod_distrib->start;
        float fs_y = evalPolynomial(fs_x, state->mod_distrib->polynomial);

        if (fs_x > fe_x)
        {
            state->mod_distrib->end = state->mod_distrib->start;
            state->mode = MOD_DISTRIBUTED_START;
            printf("Switch to MOD_DISTRIBUTED_START\n");
            break;
        }

        if (nearly_equal(fs_x, fe_x)) break;

        // Reset polynomial
        for (int i = 0; i < MAX_POLYNOMIAL_DEGREE; i++)
        {
            state->mod_distrib->polynomial[i] = 0;
        }

        line_from_points(&m, &c, fs_x, fs_y, fe_x, fe_y);
        state->mod_distrib->polynomial[0] = c;
        state->mod_distrib->polynomial[1] = m;

        state->mod_distrib->end = fe_x;

        if (mouse_released)
        {
            state->mode = NORMAL;
        }
    }; break;
    default: {
        printf("Unfinished mode: %d, entering NORMAL\n", state->mode);
        state->mode = NORMAL;
        break;
    }
    }

    // ========================= SOLVE SECTION END ========================

    SDL_RenderPresent(sdl_renderer);

    SDL_Delay(3);

    mouse_pressed = false;
    mouse_released = false;
    return true;
}


