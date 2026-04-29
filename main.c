#define JCANVAS_IMPLEMENTATION
#define JC_FAR_PLANE 10.0f
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)

#define WINX 800
#define WINY 600

#define RESX WINX
#define RESY WINY

SDL_Window *window;
SDL_Surface *window_surface;

SDL_Surface *canvas_surface;
float *zbuffer;
Canvas canvas;
Model model;
JC_Camera camera;
JC_Image image;
bool render_depth;

void update_camera(void)
{
    Vec3 forward = vec3_normalize(vec3_sub(camera.target, camera.position));
    Vec3 right = vec3_normalize(vec3_cross(forward, camera.up));
    forward = vec3_scale(forward, 0.08);
    right = vec3_scale(right, 0.08);

    // Using SDL callbacks the events pumped for us
    const bool *keys = SDL_GetKeyboardState(NULL);

    // move
    int move_dir = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
    int strafe_dir = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
    camera.position = vec3_add(camera.position, vec3_scale(forward, move_dir));
    camera.position = vec3_add(camera.position, vec3_scale(right, strafe_dir));
    if (keys[SDL_SCANCODE_SPACE]) {
        camera.position.y += 0.1;
        camera.target.y += 0.1;
    } else if (keys[SDL_SCANCODE_LCTRL]) {
        camera.position.y -= 0.1;
        camera.target.y -= 0.1;
    }

    // look
    float dx, dy;
    SDL_GetRelativeMouseState(&dx,&dy);
    float yaw = -DEG2RAD(dx) * 0.05;
    float pitch = DEG2RAD(dy) * 0.05;

    forward = vec3_transform(matrix_rotate_y(yaw), forward);
    Vec3 pitched = vec3_transform(matrix_rotate(right, pitch), forward);
    float angle = RAD2DEG(vec3_angle(camera.up, pitched));
    if (angle > 5 && angle < 175) {
        forward = pitched;
    }

    forward = vec3_normalize(forward);
    float target_dist = vec3_length(vec3_sub(camera.target, camera.position));
    camera.target = vec3_add(camera.position, vec3_scale(forward, target_dist));
}

SDL_AppResult SDL_AppIterate(void *state)
{
    (void)state;
    double frame_time_start = (double)SDL_GetTicksNS()/SDL_NS_PER_SECOND;
    update_camera();

    // Rendering
    fill(canvas, BLACK);
    // clear zbuffer
    memset(zbuffer, 0, canvas.width*canvas.height*sizeof(float));
    draw_model(canvas, camera, model, zbuffer, WHITE);
    // draw_model_wires(canvas, camera, model, GREEN);

    JC_Rect src = { 0, 0, image.width, image.height };
    int size = canvas.height / 4;
    JC_Rect dst = { canvas.width-size, canvas.height-size, size, size };
    blit_rect(canvas, image, dst, src);

    SDL_BlitSurfaceScaled(canvas_surface, NULL, window_surface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    // Limit fps
    double frame_time = ((double)SDL_GetTicksNS() / SDL_NS_PER_SECOND) - frame_time_start;
    char buf[16];
    sprintf(buf, "%dfps", (int)(1.0/frame_time)/10*10);
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

    window = SDL_CreateWindow("jcanvas", WINX, WINY, 0);
    if (window == NULL) return SDL_APP_FAILURE;
    SDL_SetWindowRelativeMouseMode(window, true);
    window_surface = SDL_GetWindowSurface(window);

    canvas_create(&canvas, RESX, RESY);
    zbuffer = calloc(canvas.width*canvas.height, sizeof(float));
    canvas_surface = SDL_CreateSurfaceFrom(canvas.width, canvas.height, SDL_PIXELFORMAT_RGBA32,
            canvas.pixels, canvas.width*sizeof(JC_Color));

    model_load(&model, "res/diablo3.obj");
    load_ppm(&model.texture, "res/diablo3_diffuse.ppm");
    load_ppm(&image, "horse.ppm");

    camera = (Camera){
        .position   = { 1.0f, 0.5f, 1.0f },
        .target     = { 0.0f, 0.0f, 0.0f },
        .up         = { 0.0f, 1.0f, 0.0f },
        .fov        = DEG2RAD(60),
        .projection = PERSPECTIVE,
    };

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    canvas_destroy(&canvas);
    model_destroy(&model);
}
