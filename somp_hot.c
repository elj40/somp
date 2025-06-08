/*
* Filename:	somp_hot.c
* Date:		Fri 06 Jun 2025 22:37:06 SAST
* Name:		EL Joubert
*
* For hot reloading somp during development
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>

#include <SDL3/SDL.h>

#define ASPECT_RATIO (16.0/9.0)
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT (WINDOW_WIDTH/ASPECT_RATIO)
#define WINDOW_TITLE "SOMP"

typedef void* module_init_t(void * state);
typedef void* module_reload_t(void * state, SDL_Window * sdl_window, SDL_Renderer * sdl_renderer);
typedef void* module_main_t(SDL_Window * sdl_window, SDL_Renderer * sdl_renderer);
typedef void* module_close_t();

typedef struct {
    module_init_t * init;
    module_reload_t * reload;
    module_main_t * main;
    module_close_t * close;
    void * state;
} SompModule;

static const char * dl_filename = "somp_gui.so";
static void * dl_handle = NULL;


bool load_module(SompModule * somp)
{
    if (dl_handle != NULL) dlclose(dl_handle);

    dl_handle = dlopen(dl_filename, RTLD_NOW);
    if (!dl_handle) {
        printf("Loading %s: %s\n", dl_filename, dlerror());
        return false;
    }

    somp->init = dlsym(dl_handle, "somp_init");
    if (!somp->init) {
        printf("Loading %s: %s\n", "somp_init", dlerror());
        return false;
    }
    somp->reload = dlsym(dl_handle, "somp_reload");
    if (!somp->reload) {
        printf("Loading %s: %s\n", "somp_reload", dlerror());
        return false;
    }
    somp->main = dlsym(dl_handle, "somp_main");
    if (!somp->main) {
        printf("Loading %s: %s\n", "somp_main", dlerror());
        return false;
    }
    somp->close = dlsym(dl_handle, "somp_close");
    if (!somp->close) {
        printf("Loading %s: %s\n", "somp_close", dlerror());
        return false;
    }
    return true;
}
int main()
{
    SDL_Window * sdl_window = NULL;
    SDL_Renderer * sdl_renderer = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) goto cleanup;
    if (
            !(sdl_window = SDL_CreateWindow(
                    WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 
                    SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALWAYS_ON_TOP)
             )
       ) goto cleanup;
    if (!(sdl_renderer = SDL_CreateRenderer(sdl_window, NULL))) goto cleanup;

    SompModule somp = {0};
    load_module(&somp);
    somp.init(somp.state);

    bool quit = false;
    bool reloaded_once = false;
    while (!quit)
    {
        const bool * key_state = SDL_GetKeyboardState(NULL);
        if (key_state[SDL_SCANCODE_H] && !reloaded_once)
        {
            somp.state = somp.close();
            if (!load_module(&somp)) quit = true;
            somp.reload(somp.state, sdl_window, sdl_renderer);
            reloaded_once = true;
        }
        else if (!key_state[SDL_SCANCODE_H]) reloaded_once = false;

        if (!somp.main(sdl_window, sdl_renderer)) quit = true;
    };

cleanup:
    SDL_Quit();
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    free(somp.state);

    return 0;
}

