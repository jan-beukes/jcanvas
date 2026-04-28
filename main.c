#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)

#define RES 160

SDL_Window *window;
SDL_Surface *window_surface;

Canvas canvas;
SDL_Surface *canvas_surface;
Model model;

void test_model_lines(void)
{
    static float theta = 0;
    theta += 0.01f;
    model.transform = jc_matrix_rotate_y(theta);

    fill(canvas, BLACK);
    draw_model(canvas, model, RED);
    for (int i = 0; i < model.vertex_count; i++) {
        Vec3 pos = vec3_transform(model.transform, model.vertices[i].position);
        Vec2 p = canvas_coord(canvas, pos);
        draw_pixel(canvas, p.x, p.y, WHITE);
    }
}

void test_triangle(void)
{
    draw_triangle(canvas, 7, 45, 35, 100, 45,  60, RED);
    draw_triangle(canvas, 120, 35, 90,   5, 45, 110, WHITE);
    draw_triangle(canvas, 115, 83, 80,  90, 85, 120, GREEN);
}

SDL_AppResult SDL_AppIterate(void *state)
{
    (void)state;
    double frame_time_start = (double)SDL_GetTicksNS()/SDL_NS_PER_SECOND;

    // test_model_lines();
    test_triangle();

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

    window = SDL_CreateWindow("jcanvas", 800, 800, 0);
    if (window == NULL) return SDL_APP_FAILURE;
    window_surface = SDL_GetWindowSurface(window);

    jc_create(&canvas, RES, RES);
    canvas_surface = SDL_CreateSurfaceFrom(canvas.width, canvas.height, SDL_PIXELFORMAT_RGBA32,
            canvas.pixels, canvas.width*sizeof(JC_Color));

    model_load(&model, "res/diablo3.obj");
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    jc_destroy(&canvas);
    model_destroy(&model);
}
