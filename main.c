#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)

#define WINX 1280
#define WINY 720

// #define RESX WINX
// #define RESY WINY
#define RESX 640
#define RESY 360

void update_camera(void);

typedef struct {
    Image diffuse;
    Image rough;
} Uniforms;

// State
SDL_Window *window;
SDL_Surface *window_surface;

SDL_Surface *canvas_surface;
float *zbuffer;
Canvas canvas;

Camera camera;
Image image;

Model floor_model, diablo, cannon, ship;

double delta_time;
bool fog_enabled = false;

bool model_shader(Vec4 *out, Vertex in, void *uniforms)
{
    Uniforms *u = uniforms;
    *out = vec4_mul(*out, image_sample(u->diffuse, in.texcoord.x, in.texcoord.y));
    *out = vec4_mul(*out, image_sample(u->rough, in.texcoord.x, in.texcoord.y));
    return true;
}

void draw_fog(void)
{
    Color fog_color = GRAY;
    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {
            int depth_y = canvas.height - 1 - y;

            float depth = 1.0f - zbuffer[depth_y*canvas.width + x];
            float near = JC_NEAR_PLANE, far = 10;
            depth = (2.0 * near) / (far + near - depth * (far - near));	
            depth = 1.0f - expf(-2*depth*depth);
            Color p = JC_PIXEL(canvas, x, y);
            Color output = color_lerp(p, fog_color, depth);
            JC_PIXEL(canvas, x, y) = output;
        }
    }
}

SDL_AppResult SDL_AppIterate(void *state)
{
    (void)state;
    static double last_frame = 0.0;
    double time_now = (double)SDL_GetTicksNS()/SDL_NS_PER_SECOND;
    delta_time = time_now - last_frame;
    last_frame = time_now;
    update_camera();

    // Rendering
    fill(canvas, BLACK);

    begin_mode_3d(canvas, camera, zbuffer);

    draw_model(cannon, (Vec3){ 1, -1, 0 }, WHITE);
    // draw_model_wires(cannon, (Vec3){ 1, -1, 0 }, GREEN);
    draw_model(diablo, (Vec3){0}, WHITE);
    draw_model(ship, (Vec3){ -5, -3, 0 }, WHITE);
    draw_model(floor_model, (Vec3){0}, WHITE);
    if (fog_enabled) draw_fog();
    end_mode_3d();

    Rect src = { 0, 0, image.width, image.height };
    int size = canvas.height / 4;
    Rect dst = { canvas.width-size, canvas.height-size, size, size };
    blit_rect(canvas, image, dst, src);

    SDL_BlitSurfaceScaled(canvas_surface, NULL, window_surface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    char buf[16];
    sprintf(buf, "%dfps", (int)(1.0/delta_time)/10*10);
    SDL_SetWindowTitle(window, buf);
    return SDL_APP_CONTINUE;
}

#define MOVE_VEL 2.0
void update_camera(void)
{
    Vec3 forward = vec3_normalize(vec3_sub(camera.target, camera.position));
    Vec3 right = vec3_normalize(vec3_cross(forward, camera.up));

    // Using SDL callbacks the events pumped for us
    const bool *keys = SDL_GetKeyboardState(NULL);

    // move
    int move_dir = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
    int strafe_dir = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
    float speed = keys[SDL_SCANCODE_LSHIFT] ? MOVE_VEL * 2 : MOVE_VEL;

    Vec3 move = { forward.x, 0, forward.z };
    move = vec3_scale(vec3_normalize(move), move_dir*speed*delta_time);
    Vec3 strafe = vec3_scale(right, strafe_dir*speed*delta_time);

    camera.position = vec3_add(camera.position, move);
    camera.position = vec3_add(camera.position, strafe);
    if (keys[SDL_SCANCODE_SPACE]) {
        camera.position.y += speed*delta_time;
        camera.target.y += speed*delta_time;
    } else if (keys[SDL_SCANCODE_LCTRL]) {
        camera.position.y -= speed*delta_time;
        camera.target.y -= speed*delta_time;
    }

    // look
    float dx, dy;
    // This will be since last time we called
    SDL_GetRelativeMouseState(&dx,&dy);
    float yaw = -DEG2RAD(dx) * 0.05;
    int yaw_dir = keys[SDL_SCANCODE_LEFT] - keys[SDL_SCANCODE_RIGHT];
    if (yaw_dir != 0) {
        yaw = yaw_dir * delta_time;
    }
    float pitch = DEG2RAD(dy) * 0.05;
    int pitch_dir = keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP];
    if (pitch_dir != 0) {
        pitch = pitch_dir * delta_time;
    }

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

SDL_AppResult SDL_AppEvent(void *state, SDL_Event *e)
{
    (void)state;
    if (e->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (e->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode k = e->key.key;
        switch (k) {
            case SDLK_ESCAPE:
                return SDL_APP_SUCCESS;
            case SDLK_TAB: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                SDL_WarpMouseInWindow(window, w/2, h/2);
                SDL_SetWindowRelativeMouseMode(window, !SDL_GetWindowRelativeMouseMode(window));
            } break;
            case SDLK_F:
                fog_enabled = !fog_enabled;
        }
    }

    return SDL_APP_CONTINUE;
}

Model create_floor(const char *path)
{
    Model model = {0};
    Vertex *v = malloc(6*sizeof(Vertex));
    float scale = 10.0f;
    v[0] = (Vertex){ .position = { -1, 0, -1 }, { .x = 0,     .y = scale }, {0}, colorf(RED) };
    v[1] = (Vertex){ .position = {  1, 0,  1 }, { .x = scale, .y = 0     }, {0}, colorf(GREEN) };
    v[2] = (Vertex){ .position = {  1, 0, -1 }, { .x = scale, .y = scale }, {0}, colorf(BLUE) };
    v[3] = (Vertex){ .position = { -1, 0, -1 }, { .x = 0,     .y = scale }, {0}, colorf(RED) };
    v[4] = (Vertex){ .position = { -1, 0,  1 }, { .x = 0,     .y = 0     }, {0}, colorf(BLUE) };
    v[5] = (Vertex){ .position = {  1, 0,  1 }, { .x = scale, .y = 0     }, {0}, colorf(GREEN) };
    
    model.vertices = v;
    model.vertex_count = 6;
    model.transform = matrix_scale(scale, scale, scale);
    model.transform = matrix_mul(matrix_translate(0, -1, 0), model.transform);
    load_ppm(&model.texture, path);
    return model;
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
            canvas.pixels, canvas.width*sizeof(Color));

    model_load(&diablo, "res/diablo3.obj");
    load_ppm(&diablo.texture, "res/diablo3_diffuse.ppm");

    model_load(&cannon, "res/cannon.obj");
    load_ppm(&cannon.texture, "res/cannon_diffuse.ppm");

    model_load(&ship, "res/ship-large.obj");
    load_ppm(&ship.texture, "res/colormap.ppm");
    ship.transform = matrix_scale(0.3, 0.3, 0.3);

    floor_model = create_floor("res/floor.ppm");

    load_ppm(&image, "res/horse.ppm");

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
    model_destroy(&diablo);
    model_destroy(&cannon);
}
