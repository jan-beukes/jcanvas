#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)

#define RESX 600
#define RESY 600

SDL_Window *window;
SDL_Surface *window_surface;

JC_Canvas canvas;
SDL_Surface *canvas_surface;
JC_Model model;

SDL_AppResult SDL_AppIterate(void *state)
{
    (void)state;
    double frame_time_start = (double)SDL_GetTicksNS()/SDL_NS_PER_SECOND;

    static float theta = 0;
    theta += 0.01f;
    model.transform = jc_matrix_rotate_y(theta);

    jc_fill(canvas, BLACK);
    jc_model(canvas, model, RED);
    for (int i = 0; i < model.vertex_count; i++) {
        JC_Vec3 pos = jc_vec3_transform(model.transform, model.vertices[i].position);
        JC_Vec2 p = jc_canvas_coord(canvas, pos);
        jc_pixel(canvas, p.x, p.y, WHITE);
    }

    SDL_BlitSurfaceScaled(canvas_surface, NULL, window_surface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    // Limit fps
    double frame_time = ((double)SDL_GetTicksNS() / SDL_NS_PER_SECOND) - frame_time_start;
    if (frame_time < FRAME_TIME) {
        SDL_DelayNS(FRAME_TIME - frame_time);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *state, SDL_Event *e)
{
    (void)state;

    if (e->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE) return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **state, int argc, char *argv[])
{
    (void)state, (void)argc, (void)argv;

    window = SDL_CreateWindow("jcanvas", RESX, RESY, 0);
    if (window == NULL) return SDL_APP_FAILURE;
    window_surface = SDL_GetWindowSurface(window);

    jc_create(&canvas, RESX, RESY);
    canvas_surface = SDL_CreateSurfaceFrom(canvas.width, canvas.height, SDL_PIXELFORMAT_RGBA32,
            canvas.pixels, canvas.width*sizeof(JC_Color));

    jc_model_load(&model, "res/diablo3.obj");
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    jc_destroy(&canvas);
    jc_model_destroy(&model);
}
