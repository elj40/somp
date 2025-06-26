#include <stdbool.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#define SOMP_LOGIC_IMPLEMENTATION
#include "somp_logic.h"

#define UTIlS_IMPLEMENTATION
#include "utils.h"

#define somp_loginfo(cat, msg) SDL_LogInfo((cat), (msg))

#define COLOR_DEFAULT         COLOR_BLACK
#define COLOR_PREVIEW         COLOR_GRAY
#define COLOR_HIGHLIGHT       COLOR_RED
#define COLOR_RED             255,   0,   0, 255
#define COLOR_BLACK             0,   0,   0, 255
#define COLOR_GRAY            100, 100, 100, 255
#define COLOR_HIBB_BACKGROUND 255, 252, 233, 255
#define COLOR_HIBB_BEAM        79, 167, 195, 255
#define EJSDL_COLOR(color) (SDL_Color){ color }
#define EXPAND_COLOR(color) color.r, color.g, color.b, color.a

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
        SompPointForce temp_point_force;
        SompDistrForce temp_distr_force;
    };
    bool distributed_first_placed;

    // TODO: magic number
    // Also this is just a TERRIBLE idea, we should look into a temporary
    // allocator instead
    float temp_floats[10];

    SolveMode mode;
} somp_section_solve_t;

typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;

    somp_section_solve_t solve;
} SompState;


SompState * somp_state;
SompGui gui = {0};

void gui_init(SompGui * const gui);

// ==================== HOT RELOAD FUNCS ================================
bool somp_init(void * state, SDL_Window * window, SDL_Renderer * renderer, ...)
{
    if (!state) somp_state = malloc(sizeof(SompState));
    else somp_state = state;

    somp_state->window = window;
    somp_state->renderer = renderer;

    //DynamicArrayAppend(&somp_state->solve.point_forces, ((PointForce){ .distance=0.000000, .force=38.728027 }));

    somp_state->solve.beam.length = 1.0;
    somp_state->solve.beam.sections_count = MAX_SECTIONS;
    gui_init(&gui);

    return true;
};
bool somp_reload(void * state)
{
    if (!state)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Received invalid state. Run somp_init before reload\n");
        return false;
    } else somp_state = state;

    somp_loginfo(SDL_LOG_CATEGORY_APPLICATION, "Hot Reload\n");
    gui_init(&gui);

    return true;
};
SompState * somp_close()
{
    return somp_state;
};
// ======================================================================
// ====================== SOLVE SECTION =================================
#define swap(x,y,type) do { type t = (x); x = y; y = t; } while(0)
float px_to_force(float y, SompBoundary bound)
{
    // For now we assume that the beam or axis we care about is exaclty in the middle of the boundary
    // We use the y component and map it linearly, modify this later
    // Also assuming 1px = 1N

    //f = Y+H/2 - y
    return bound.y+bound.h/2 - y;
};
bool hover(const SompBoundary bound)
{
    SDL_FPoint mouse = { gui.mouse_x, gui.mouse_y };
    //SDL_RenderRect(somp_state->renderer, &bound);
    return SDL_PointInRectFloat(&mouse, &bound);
    //SDL_RenderRect(somp_state->renderer, &rect);
    //int mx = gui.mouse_x, my = gui.mouse_y;
    //return (x <= mx && mx <= x+w) && (y <= my && my <= y+h);
}
float force_to_px(float force, SompBoundary bound)
{
    // This should be the inverse of px_to_force;

    // f = Y+H/2 - y
    // y = Y+H/2 - f
    return bound.y+bound.h/2 - force;
};
void clear_background(SompBoundary boundary, SDL_Color color)
{
    SDL_SetRenderDrawColorFloat(somp_state->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(somp_state->renderer, &boundary);
};
void render_arrow_vert(SDL_Renderer * sdl_renderer,
        float x, float y1, float y2, float head_width)
{
    float arrow_head_length = (y2 < y1) ? -head_width : head_width;

    SDL_RenderLine(sdl_renderer, x, y1, x, y2);
    SDL_RenderLine(sdl_renderer, x + head_width, y2 - arrow_head_length, x, y2);
    SDL_RenderLine(sdl_renderer, x - head_width, y2 - arrow_head_length, x, y2);
}
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
void render_point_force(SompBoundary beam_bound, SompBeam beam, SompPointForce pf, SDL_Color color)
{
    // TODO: magic numbers
    const int hl_distance = 4;
    // Assumptions in this function
    //  1. Beam rendered in centre of beam_bound
    //  2. Beam width = 10
    //  3. Arrow head width is same everywhere
    int arrow_x  = lerp(beam_bound.x, beam_bound.x+beam_bound.w, invlerp(0, beam.length, pf.distance)); // 1.
    int arrow_ys = (pf.force > 0) ? beam_bound.y : beam_bound.y+beam_bound.h; // 1.
    int arrow_ye = beam_bound.y + beam_bound.h/2 + ((pf.force > 0) ? 0 : 10);   // 2.

    SDL_FRect hover_rect = {
        arrow_x - hl_distance,
        minf(arrow_ys, arrow_ye),
        hl_distance*2,
        abs(arrow_ye - arrow_ys)
    };
    if (hover(hover_rect))
        SDL_SetRenderDrawColor(somp_state->renderer, COLOR_HIGHLIGHT);
    else
        SDL_SetRenderDrawColor(somp_state->renderer, color.r, color.g, color.b, color.a);
    render_arrow_vert(somp_state->renderer, arrow_x, arrow_ys, arrow_ye, 5); // 3.
};

void render_point_forces(const SompBoundary beam_bound, const SompBeam beam, const SompPointForces point_forces)
{
    for (int i = 0; i < point_forces.count; i++)
    {
        render_point_force(beam_bound, beam, point_forces.items[i], EJSDL_COLOR(COLOR_DEFAULT));
    };
}
// TODO: I want polynomial parameter to be const but evalPolynomial takes a
// mutable array, this should be changed
// Returns absolute pixel values of the heights that the distributed lines should be drawns
// Has some magic numbers
void get_distr_line_heights(float * const ys, float * const ye, const SompBoundary beam_bound, float polynomial[], const float dist)
{
    // TODO: magic numbers
    // Assumptions in this function
    //  1. Beam rendered in centre of beam_bound
    //  2. Beam width = 10
    float px_offset = force_to_px(evalPolynomial(dist, polynomial), beam_bound);
    int line_ys = px_offset; // 1.
    int line_ye;
    if (px_offset < beam_bound.y + beam_bound.h/2) line_ye = beam_bound.y + beam_bound.h/2; // 1 2.
    else if (px_offset < beam_bound.y + beam_bound.h/2 + 10) line_ye = line_ys;
    else line_ye = beam_bound.y + beam_bound.h/2 + 10; // 1 2.

    if (ys != NULL) *ys = line_ys;
    if (ye != NULL) *ye = line_ye;
};
void render_phony_distr_force(SompBoundary beam_bound,
        float xs, float ys, float xe, float ye,
        SDL_Color color)
{
    // TODO: magic number
    const int distr_preview_step = 16;


    float temp_poly[MAX_POLYNOMIAL_DEGREE] = {0};
    line_from_points(&temp_poly[1], &temp_poly[0], xs, ys, xe, ye);

    SDL_SetRenderDrawColor(somp_state->renderer, color.r, color.g, color.b, color.a);

    float line_ys, line_ye;
    get_distr_line_heights(&line_ys, &line_ye, beam_bound, temp_poly, xs);
    SDL_RenderLine(somp_state->renderer, xs, line_ys, xs, line_ye);

    float xp = xs, yp = line_ys;

    xs += distr_preview_step - ((int)xs % distr_preview_step);
    for (int x = xs; x <= xe; x += distr_preview_step)
    {

        get_distr_line_heights(&line_ys, &line_ye, beam_bound, temp_poly, x);
        SDL_RenderLine(somp_state->renderer, x, line_ys, x, line_ye);
        SDL_RenderLine(somp_state->renderer, x, line_ys, xp, yp);

        xp = x;
        yp = line_ys;
    }

    get_distr_line_heights(&line_ys, &line_ye, beam_bound, temp_poly, xe);
    SDL_RenderLine(somp_state->renderer, xe, line_ys, xp, yp);
    SDL_RenderLine(somp_state->renderer, xe, line_ys, xe, line_ye);
}
void distr_side(SDL_FRect hover_rect, SDL_Color default_color, SDL_Color hl_color)
{
    if (hover(hover_rect)) {
        SDL_SetRenderDrawColor(somp_state->renderer, EXPAND_COLOR(hl_color));
    }
    else SDL_SetRenderDrawColor(somp_state->renderer, EXPAND_COLOR(default_color));

}
void render_distr_force(SompBoundary beam_bound, SompBeam beam, SompDistrForce df, SDL_Color color)
{
    // TODO: magic number
    const int distr_preview_step = 16;
    const int hl_dist_x = 4;
    //const int hl_dist_y = 4;

    int x_start = lerp(beam_bound.x, beam_bound.x+beam_bound.w, invlerp(0, beam.length, df.start));
    int x_end   = lerp(beam_bound.x, beam_bound.x+beam_bound.w, invlerp(0, beam.length, df.end  ));

    float line_ys, line_ye;
    get_distr_line_heights(&line_ys, &line_ye, beam_bound, df.polynomial, df.start);

    // Start
    SompBoundary hover_rect;
    hover_rect = (SompBoundary){
        x_start-hl_dist_x,
        minf(line_ys, line_ye),
        hl_dist_x*2,
        fabsf(line_ye-line_ys)
    };
    distr_side(hover_rect, color, EJSDL_COLOR(COLOR_HIGHLIGHT));
    SDL_RenderLine(somp_state->renderer, x_start, line_ys, x_start, line_ye);

    SDL_SetRenderDrawColor(somp_state->renderer, color.r, color.g, color.b, color.a);

    float xp = x_start, yp = line_ys;

    x_start += distr_preview_step - (x_start % distr_preview_step);
    for (int x = x_start; x <= x_end; x += distr_preview_step)
    {
        float dist = lerp(0, beam.length, invlerp(beam_bound.x, beam_bound.x+beam_bound.w, x));
        get_distr_line_heights(&line_ys, &line_ye, beam_bound, df.polynomial, dist);
        SDL_RenderLine(somp_state->renderer, x, line_ys, xp, yp);
        SDL_RenderLine(somp_state->renderer,
                x, line_ys,
                x, line_ye);

        xp = x;
        yp = line_ys;
    }

    get_distr_line_heights(&line_ys, &line_ye, beam_bound, df.polynomial, df.end);
    SDL_RenderLine(somp_state->renderer, x_end, line_ys, xp, yp);

    // End
    hover_rect = (SompBoundary){
        x_end-hl_dist_x,
        minf(line_ys, line_ye),
        hl_dist_x*2,
        fabsf(line_ye-line_ys)
    };
    distr_side(hover_rect, color, EJSDL_COLOR(COLOR_HIGHLIGHT));
    SDL_RenderLine(somp_state->renderer, x_end, line_ys, x_end, line_ye);

}
void render_distr_forces(SompBoundary beam_bound, SompBeam beam, SompDistrForces distr_forces)
{
    for (int i = 0; i < distr_forces.count; i++)
    {
        render_distr_force(beam_bound, beam, distr_forces.items[i], EJSDL_COLOR(COLOR_DEFAULT));
    };
}
bool normal(SompBoundary beam_bound, const SompPointForces * point_forces, const SompDistrForces * distr_forces) 
{
    (void)beam_bound;
    (void)point_forces;
    (void)distr_forces;

    return true;
}
bool add_point_force(SompBoundary beam_bound, const SompBeam beam, SompPointForces * point_forces)
{
    SompPointForce new_force = {0};

    float new_force_x = MIN(beam_bound.x+beam_bound.w, MAX(beam_bound.x, gui.mouse_x));
    float new_force_y = gui.mouse_y;

    new_force.force    = px_to_force(new_force_y, beam_bound);
    new_force.distance = lerp(0, beam.length, invlerp(beam_bound.x, beam_bound.x+beam_bound.w, new_force_x));


    render_point_force(beam_bound, beam, new_force, EJSDL_COLOR(COLOR_PREVIEW));

    if (gui.mouse_pressed && gui.mouse_state == SDL_BUTTON_LEFT)
    {
        printf("Adding point force\n");
        DynamicArrayAppend(point_forces, new_force);
    };


    return true;
}
bool add_distr_force(SompBoundary beam_bound, SompBeam beam, SompDistrForces * distr_forces)
{
    somp_section_solve_t * S = &somp_state->solve;
    // TODO: magic number
    const float preview_width = beam_bound.w*0.2;

    float * xs, * ys;
    float * xe, * ye;

    xs = &S->temp_floats[0];
    xe = &S->temp_floats[1];
    ys = &S->temp_floats[2];
    ye = &S->temp_floats[3];

    if (!S->distributed_first_placed)
    {
        *xs = minf(beam_bound.x+beam_bound.w, maxf(beam_bound.x, gui.mouse_x));
        *xe = minf(beam_bound.x+beam_bound.w, maxf(beam_bound.x, gui.mouse_x)+preview_width);

        *ys = beam_bound.y + beam_bound.h/2 - gui.mouse_y;
        *ye = beam_bound.y + beam_bound.h/2 - gui.mouse_y;

        if (gui.mouse_pressed && gui.mouse_state == SDL_BUTTON_LEFT)
        {
            S->distributed_first_placed = true;
        }
    } else
    {
        *xe = MIN(beam_bound.x+beam_bound.w, MAX(beam_bound.x, gui.mouse_x));
        *ye = beam_bound.y + beam_bound.h/2 - gui.mouse_y;

        if (*xs > *xe)
        {
            swap(xs, xe, float *);
            swap(ys, ye, float *);
        }

        if (gui.mouse_released)
        {
            float fxs, fys, fxe, fye;
            fxs = lerp(0, beam.length, invlerp(beam_bound.x, beam_bound.x+beam_bound.w, *xs));
            fxe = lerp(0, beam.length, invlerp(beam_bound.x, beam_bound.x+beam_bound.w, *xe));

            float ys_abs, ye_abs;
            ys_abs = beam_bound.y + beam_bound.h/2 - *ys;
            ye_abs = beam_bound.y + beam_bound.h/2 - *ye;
            fys = px_to_force(ys_abs, beam_bound);
            fye = px_to_force(ye_abs, beam_bound);

            SompDistrForce df = {
                .start = fxs,
                .end   = fxe,
            };
            float m, c;
            line_from_points(&m, &c, fxs, fys, fxe, fye);

            df.polynomial[0] = c;
            df.polynomial[1] = m;

            DynamicArrayAppend(distr_forces, df);
            S->distributed_first_placed = false;
            S->mode = NORMAL;

            printf("%f %d\n", beam.length, beam.sections_count);
            printDF(&df);
            printf("Adding distributed force\n");
        }

    }

    render_phony_distr_force(beam_bound, *xs, *ys, *xe, *ye, EJSDL_COLOR(COLOR_PREVIEW));
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
    render_point_forces(beam_boundary, *beam, *point_forces);
    render_distr_forces(beam_boundary, *beam, *distr_forces);

    switch (state->mode) {
    case NORMAL:                  normal(beam_boundary, point_forces, distr_forces); break;
    case ADD_POINT_FORCE:         add_point_force(beam_boundary, *beam, point_forces); break;
    case ADD_DISTRIB_FORCE:       add_distr_force(beam_boundary, *beam, distr_forces); break;
    case MOD_POINT_FORCE:         mod_point_force(beam_boundary, &state->temp_point_force); break;
    case MOD_DISTR_FORCE_START:   mod_distr_force_start(beam_boundary, &state->temp_distr_force); break;
    case MOD_DISTR_FORCE_END:     mod_distr_force_end  (beam_boundary, &state->temp_distr_force); break;
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

void gui_init(SompGui * const gui)
{
    SDL_GetWindowSize(somp_state->window, &gui->windoww, &gui->windowh);
    gui->mouse_state = SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);


}
void gui_update(SDL_Event e, SompGui * const gui)
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

void gui_reset(SompGui * const gui)
{
    gui->mouse_pressed = false;
    gui->mouse_released = false;
}

void keyboard_shortcuts(SDL_Event e)
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
        gui_update(sdl_event, &gui);
        switch(sdl_event.type)
        {
        case SDL_EVENT_QUIT: return false;
        case SDL_EVENT_KEY_DOWN: keyboard_shortcuts(sdl_event); break;
        }
    }
    SompBoundary boundary_solve = { 0, 0, gui.windoww, gui.windowh };
    SompBoundary boundary_2 = { 0, 0, 0, 0 };
    SompBoundary boundary_3 = { 0, 0, 0, 0 };


    if (!somp_section_solve(boundary_solve)) return false;
    if (!somp_section_2(boundary_2)) return false;
    if (!somp_section_3(boundary_3)) return false;

    gui_reset(&gui);

    SDL_Delay(32);
    return true;
}
