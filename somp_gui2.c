#include <stdbool.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

#define UTIlS_IMPLEMENTATION
#include "utils.h"

#define somp_loginfo(cat, msg) SDL_LogInfo((cat), (msg))

#define COLOR_DEFAULT         COLOR_BLACK
#define COLOR_RED             255,   0,   0, 255
#define COLOR_BLACK             0,   0,   0, 255
#define COLOR_GRAY            100, 100, 100, 255
#define COLOR_HIBB_BACKGROUND 255, 252, 233, 255
#define COLOR_HIBB_BEAM        79, 167, 195, 255
#define EJSDL_COLOR(color) (SDL_Color){ color }

typedef SDL_FRect             SompBoundary;
typedef Beam                  SompBeam;
typedef PointForces           SompPointForces;
typedef DistributedForces     SompDistrForces;
typedef PointForce            SompPointForce;
typedef DistributedForce      SompDistrForce;

typedef struct {
    int windoww;
    int windowh;

    // Mouse
    float mouse_x;
    float mouse_y;
    bool mouse_pressed;
    bool mouse_released;
    bool mouse_down;
    SDL_MouseButtonFlags mouse_state;
} SompGui;

typedef enum {
    NORMAL,
    ADD_POINT_FORCE,
    ADD_DISTRIB_FORCE,
    MOD_POINT_FORCE,
    MOD_DISTR_FORCE_START,
    MOD_DISTR_FORCE_END,
} SolveMode;

typedef struct {
    SompBeam beam;
    SompPointForces point_forces;
    SompDistrForces distr_forces;

    union {
        SompPointForce * mod_point_force;
        SompDistrForce * mod_distr_force;
    };
    bool distributed_first_placed;

    SolveMode mode;
} somp_section_solve_t;

typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;

    somp_section_solve_t solve;
} SompState;


SompState * somp_state;
SompGui gui = {0};

// ==================== HOT RELOAD FUNCS ================================

bool somp_init(void * state, SDL_Window * window, SDL_Renderer * renderer, ...)
{
    if (!state) somp_state = malloc(sizeof(SompState));
    else somp_state = state;

    somp_state->window = window;
    somp_state->renderer = renderer;

    return true;
};
bool somp_reload(void * state)
{
    if (!state)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Received invalid state. Run somp_init before reload\n");
        return false;
    } else somp_state = state;

    return true;
};
SompState * somp_close()
{
    return somp_state;
};
// ======================================================================
// ====================== SOLVE SECTION =================================
void clear_background(SompBoundary boundary, SDL_Color color)
{
    SDL_SetRenderDrawColorFloat(somp_state->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(somp_state->renderer, &boundary);
};

void render_beam(SompBoundary beam_bound)
{
    SDL_FRect beam_rect = { beam_bound.x, beam_bound.y + 0.5*beam_bound.h, beam_bound.w, 10 };
    SDL_SetRenderDrawColor(somp_state->renderer, COLOR_HIBB_BEAM);
    SDL_RenderFillRect(somp_state->renderer, &beam_rect);
    SDL_SetRenderDrawColor(somp_state->renderer, COLOR_BLACK);
    SDL_RenderRect(somp_state->renderer, &beam_rect);

    SDL_SetRenderDrawColor(somp_state->renderer, COLOR_BLACK);
    SDL_RenderLine(somp_state->renderer, beam_bound.x, beam_bound.y, beam_rect.x, beam_bound.y+beam_bound.h);
};

void render_point_forces(SompBoundary beam_bound, SompBeam * beam, SompPointForces * point_forces) 
{
   (void)beam_bound;
   (void)beam;
   (void)point_forces;
}
void render_distr_forces(SompBoundary beam_bound, const SompBeam * beam, const SompDistrForces * distr_forces) 
{
   (void)beam_bound;
   (void)beam;
   (void)distr_forces;
}
bool normal(SompBoundary beam_bound, const SompPointForces * point_forces, const SompDistrForces * distr_forces) 
{
    (void)beam_bound;
    (void)point_forces;
    (void)distr_forces;

    return true;
}
float px_to_force(float y, SompBoundary bound)
{
    // For now we assume that the beam or axis we care about is exaclty in the middle of the boundary
    // We use the y component and map it linearly, modify this later
    // Also assuming 1px = 1N
    return bound.y+bound.h/2 - y;
};
bool add_point_force(SompBoundary beam_bound, const SompBeam beam, SompPointForces * point_forces)
{
    SompPointForce new_force = {0};

    float new_force_x = MIN(beam_bound.x+beam_bound.w, MAX(beam_bound.x, gui.mouse_x));
    float new_force_y = MIN(beam_bound.y+beam_bound.h, MAX(beam_bound.y, gui.mouse_y));

    new_force.force    = px_to_force(new_force_y, beam_bound);
    new_force.distance = lerp(0, beam.length, invlerp(beam_bound.x, beam_bound.x+beam_bound.w, new_force_x));

    if (gui.mouse_pressed && gui.mouse_state == SDL_BUTTON_LEFT)
    {
        printf("Adding point force\n");
        DynamicArrayAppend(point_forces, new_force);
    };

    return true;
}
bool add_distr_force(SompBoundary beam_bound, SompDistrForces * distr_forces)
{
    (void)beam_bound;
    (void)distr_forces;

    return true;
}
bool mod_point_force(SompBoundary beam_bound, SompPointForce * point_force)
{
    (void)beam_bound;
    (void)point_force;

    return true;
}
bool mod_distr_force_start(SompBoundary beam_bound, SompDistrForce * distr_force)
{
    (void)beam_bound;
    (void)distr_force;

    return true;
}
bool mod_distr_force_end(SompBoundary beam_bound, SompDistrForce * distr_force)
{
    (void)beam_bound;
    (void)distr_force;

    return true;
}

bool somp_section_solve(SompBoundary boundary)
{
    somp_section_solve_t * state = &somp_state->solve;

    SDL_Color background_color = EJSDL_COLOR(COLOR_HIBB_BEAM);
    clear_background(boundary, background_color);

    SompBoundary beam_boundary = { 0.1*gui.windoww, 0.1*gui.windowh, 0.8*gui.windoww, 0.8*gui.windowh };
    SompBeam * beam = &state->beam;
    SompPointForces * point_forces = &state->point_forces;
    SompDistrForces * distr_forces = &state->distr_forces;

    render_beam(beam_boundary);
    render_point_forces(beam_boundary, beam, point_forces);
    render_distr_forces(beam_boundary, beam, distr_forces);

    switch (state->mode) {
    case NORMAL:                  normal(beam_boundary, point_forces, distr_forces); break;
    case ADD_POINT_FORCE:         add_point_force(beam_boundary, *beam, point_forces); break;
    case ADD_DISTRIB_FORCE:       add_distr_force(beam_boundary, distr_forces); break;
    case MOD_POINT_FORCE:         mod_point_force(beam_boundary, state->mod_point_force); break;
    case MOD_DISTR_FORCE_START:   mod_distr_force_start(beam_boundary, state->mod_distr_force); break;
    case MOD_DISTR_FORCE_END:     mod_distr_force_end  (beam_boundary, state->mod_distr_force); break;
    default: {
        somp_loginfo(SDL_LOG_CATEGORY_APPLICATION, "Unknown mode, switching to NORMAL\n");
        state->mode = NORMAL;
    }
    };

    SDL_RenderPresent(somp_state->renderer);
    return true;
}
// ======================================================================
bool somp_section_2(SompBoundary boundary) { (void)boundary; return true; }
bool somp_section_3(SompBoundary boundary) { (void)boundary; return true;}

void update_gui(SDL_Event e, SompGui * const gui)
{
    switch(e.type)
    {
    case SDL_EVENT_WINDOW_SHOWN:
    case SDL_EVENT_WINDOW_RESIZED: {
        int w,h;
        SDL_GetWindowSize(somp_state->window, &w, &h);
        gui->windoww = w;
        gui->windowh = h;
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
        gui->mouse_state = SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);
        gui->mouse_released = true;
        gui->mouse_down = false;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        gui->mouse_state = SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);
        gui->mouse_pressed = true;
        gui->mouse_down = true;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        gui->mouse_state = SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);
        break;
    };
};

void reset_gui(SompGui * const gui)
{
    gui->mouse_pressed = false;
    gui->mouse_released = false;
}

void keyboard_shortcuts(SDL_Event e, SompGui * const gui)
{
    somp_section_solve_t * S = &somp_state->solve;
    switch(e.key.key)
    {
    case SDLK_S: {
        solveBeam(&S->beam,
                S->point_forces.items, S->point_forces.count,
                S->distr_forces.items, S->distr_forces.count);
        printf("Beam: {\n");
        printf("\t.length = %f\n", S->beam.length);
        printf("\t.sections_count = %d\n", S->beam.sections_count);
        printf("Raw: ");
        printStructArray(S->beam.raws, S->beam.sections_count, sizeof(Section), printSection);
        printf("shear: ");
        printStructArray(S->beam.shears, S->beam.sections_count, sizeof(Section), printSection);
        printf("moment: ");
        printStructArray(S->beam.moments, S->beam.sections_count, sizeof(Section), printSection);
        printf("}\n");
    }; break;
    case SDLK_F: {
        S->mode = (S->mode == ADD_POINT_FORCE) ? NORMAL : ADD_POINT_FORCE;
    }; break;
    case SDLK_D: {
        S->mode = (S->mode == ADD_DISTRIB_FORCE) ? NORMAL : ADD_DISTRIB_FORCE;
        S->distributed_first_placed = false;
    }; break;
    case SDLK_R: {
        S->point_forces.count = 0;
        S->distr_forces.count = 0;
    }; break;
    }
};

// Main function that controls logic of application
// Returns: true to continue looping
//          false to exit application
bool somp_main()
{
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        update_gui(sdl_event, &gui);
        switch(sdl_event.type)
        {
        case SDL_EVENT_QUIT: return false;
        case SDL_EVENT_KEY_DOWN: keyboard_shortcuts(sdl_event, &gui); break;
        }
    }
    SompBoundary boundary_solve = { 0, 0, gui.windoww, gui.windowh };
    SompBoundary boundary_2 = { 0, 0, 0, 0 };
    SompBoundary boundary_3 = { 0, 0, 0, 0 };


    if (!somp_section_solve(boundary_solve)) return false;
    if (!somp_section_2(boundary_2)) return false;
    if (!somp_section_3(boundary_3)) return false;

    reset_gui(&gui);
    return true;
}
