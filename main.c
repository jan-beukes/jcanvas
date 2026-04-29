#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)

#define RES 800

SDL_Window *window;
SDL_Surface *window_surface;

SDL_Surface *canvas_surface;
float *zbuffer;
Canvas canvas;
Model model;
JC_Camera camera;
JC_Image image;
bool render_depth;

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

    Vec3 forward = vec3_normalize(vec3_sub(camera.target, camera.position));
    Vec3 right = vec3_normalize(vec3_cross(forward, camera.up));
    forward = vec3_scale(forward, 0.08);
    right = vec3_scale(right, 0.08);

    // Using SDL callbacks the events pumped for us
    const bool *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W]) {
        camera.position = vec3_add(camera.position, forward);
    } else if (keys[SDL_SCANCODE_S]) {
        camera.position = vec3_sub(camera.position, forward);
    } else if (keys[SDL_SCANCODE_A]) {
        camera.position = vec3_sub(camera.position, right);
    } else if (keys[SDL_SCANCODE_D]) {
        camera.position = vec3_add(camera.position, right);
    }
    if (keys[SDL_SCANCODE_SPACE]) {
        camera.position.y += 0.1;
        camera.target.y += 0.1;
    } else if (keys[SDL_SCANCODE_LCTRL]) {
        camera.position.y -= 0.1;
        camera.target.y -= 0.1;
    }

    float dx, dy;
    SDL_GetRelativeMouseState(&dx,&dy);
    float yaw = -DEG2RAD(dx) * 0.1;
    float pitch = DEG2RAD(dy) * 0.1;

    forward = vec3_transform(matrix_rotate_y(yaw), forward);
    forward = vec3_transform(matrix_rotate(right, pitch), forward);
    forward = vec3_normalize(forward);
    float target_dist = vec3_length(vec3_sub(camera.target, camera.position));
    camera.target = vec3_add(camera.position, vec3_scale(forward, target_dist));


    // Rendering
    fill(canvas, BLACK);
    // clear zbuffer
    memset(zbuffer, 0, canvas.width*canvas.height*sizeof(float));
    draw_model(canvas, camera, model, zbuffer, WHITE);

    JC_Rect src = { 0, 0, image.width, image.height };
    JC_Rect dst = { canvas.width - 200, canvas.height - 200, 200, 200 };
    blit_rect(canvas, image, dst, src);

    SDL_BlitSurfaceScaled(canvas_surface, NULL, window_surface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    // Limit fps
    double frame_time = ((double)SDL_GetTicksNS() / SDL_NS_PER_SECOND) - frame_time_start;
    char buf[16];
    sprintf(buf, "%.2fms", frame_time * 1000);
    SDL_SetWindowTitle(window, buf);
    if (frame_time < FRAME_TIME) {
        double sleep_time = (FRAME_TIME - frame_time) * SDL_NS_PER_SECOND;
        SDL_DelayNS(sleep_time);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *state, SDL_Event *e)
{
    (void)state;
    if (e->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (e->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode k = e->key.key;
        if (k == SDLK_ESCAPE) return SDL_APP_SUCCESS;
        if (k == SDLK_TAB) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            SDL_WarpMouseInWindow(window, w/2, h/2);
            SDL_SetWindowRelativeMouseMode(window, !SDL_GetWindowRelativeMouseMode(window));
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **state, int argc, char *argv[])
{
    (void)state, (void)argc, (void)argv;

    window = SDL_CreateWindow("jcanvas", 800, 800, 0);
    if (window == NULL) return SDL_APP_FAILURE;
    window_surface = SDL_GetWindowSurface(window);

    canvas_create(&canvas, RES, RES);
    zbuffer = calloc(canvas.width*canvas.height, sizeof(float));
    canvas_surface = SDL_CreateSurfaceFrom(canvas.width, canvas.height, SDL_PIXELFORMAT_RGBA32,
            canvas.pixels, canvas.width*sizeof(JC_Color));

    model_load(&model, "res/diablo3.obj");
    load_ppm(&model.texture, "res/diablo3_diffuse.ppm");
    load_ppm(&image, "horse.ppm");

    camera = (Camera){
        .position   = { 1.0f, 1.0f, 1.0f },
        .target     = { 0.0f, 0.0f, 0.0f },
        .up         = { 0.0f, 1.0f, 0.0f },
        .fov        = 60,
        .projection = JC_PERSPECTIVE,
    };

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    canvas_destroy(&canvas);
    model_destroy(&model);
}
