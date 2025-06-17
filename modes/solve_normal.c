#ifndef _SOMP_NORMAL_M_
#define _SOMP_NORMAL_M_
#define HIGHLIGHT_DISTANCE 5

#define UTILS_IMPLEMENTATION
#include "../utils.h"

void somp_solve_normal(SDL_Renderer * sdl_renderer, SDL_FRect beam_rect)
{
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
#endif // _SOMP_NORMAL_M_
