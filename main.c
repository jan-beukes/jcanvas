#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define FRAME_TIME (1.0 / 60.0)
#define GRAVITY 10.0f
#define PLAYER_HEIGHT 1.4f

#define WINX 1280
#define WINY 720

// #define RESX WINX
// #define RESY WINY
#define RESX 640
#define RESY 360

#define LIGHT_COUNT 3
#define DIABLO_COUNT 3

void update_player(void);

// STATE
SDL_Window *window;
SDL_Surface *window_surface;
SDL_Surface *canvas_surface;

float *zbuffer;
Canvas canvas;

Color white_pixel[1] = { WHITE };
Image white_image = {
    .pixels = white_pixel,
    .width = 1, .height = 1,
};

Model building;
Model floor_model, diablo, cannon;
JC_Vec3 diablo_positions[DIABLO_COUNT];

bool fog_enabled = false;
double delta_time;

Camera camera;
struct {
    Vec3 pos;
    Vec3 vel;
    Vec3 look_dir;
    bool flying;
} player;


typedef struct {
    Vec3 position;
    float strength;
    Vec4 color;
} Light;

#define LIGHT_ATTENUATION 2.0f
#define AMBIENT (Vec4){ 0.3f, 0.2f, 0.2f, 1.0f }
Light lights[LIGHT_COUNT] = {
    { { -10, 3, 0 }, 50.0f, { 0.9f, 1.0f, 0.9f, 1.0f } },
    { { 0,   3, 0 }, 50.0f, { 1.0f, 0.9f, 0.9f, 1.0f } },
    { { 10,  3, 0 }, 50.0f, { 0.9f, 0.9f, 1.0f, 1.0f } },
};

bool shader(Vec4 *out, Vertex in, Vec3 pos, void *uniforms)
{
    Image *image = uniforms;
    Vec4 img_color = image_sample(*image, in.texcoord.x, in.texcoord.y);

    Vec3 normal = vec3_normalize(in.normal);
    Vec4 diffuse = {0};
    for (int i = 0; i < LIGHT_COUNT; i++) {
        Vec3 light_vec = vec3_sub(lights[i].position, pos);
        float dist_sqr = vec3_dot(light_vec, light_vec);
        float dot = vec3_dot(normal, vec3_normalize(light_vec));
        dot = MAX(dot, 0);
        dist_sqr *= LIGHT_ATTENUATION * LIGHT_ATTENUATION;
        float dist_inv = 1.0f / dist_sqr;
        Vec4 l = vec4_scale(lights[i].color, dot * lights[i].strength * dist_inv);
        diffuse = vec4_add(diffuse, l);
    }
    diffuse.w = 1.0f;
    JC_Vec4 light = vec4_add(AMBIENT, diffuse);
    *out = vec4_mul(light, img_color);
    return true;
}

void draw_fog(void)
{
    Color fog_color = DARKGRAY;
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
    update_player();

    static float rotation = 0.0f;
    rotation += 0.1f*delta_time;

    // Rendering
    camera.position = vec3_add(player.pos, (Vec3){ 0, PLAYER_HEIGHT, 0 });
    camera.target = vec3_add(camera.position, player.look_dir);
    begin_mode_3d(canvas, camera, zbuffer);

    fill(canvas, colorb(AMBIENT)); 
    set_shader(shader, &building.texture);
    disable_backface_culling();
    draw_model(building, (Vec3){0}, WHITE);
    enable_backface_culling();

    set_shader_uniforms(&diablo.texture);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        Vec3 light_xz = { lights[i].position.x, 0, lights[i].position.z };
        for (int j = 0; j < DIABLO_COUNT; j++){
            Vec3 pos = diablo_positions[j];
            Matrix translate = matrix_translate(pos.x, pos.y, pos.z);
            diablo.transform = matrix_mul(matrix_rotate_y(rotation), translate);
            draw_model(diablo, light_xz, WHITE);
        }
    }

    set_shader_uniforms(&cannon.texture);
    draw_model(cannon, (Vec3){ 1, 0, 0 }, WHITE);

    set_shader_uniforms(&floor_model.texture);
    draw_model(floor_model, (Vec3){0}, WHITE);
    // LIGHTS
    set_shader_uniforms(&white_image);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        Vec3 lpos = lights[i].position;
        Vec3 p = { lpos.x, 0.5*lpos.y, lpos.z };
        jc_draw_cube(p, (Vec3){ 0.1f, lpos.y, 0.1f }, color_from_int(0x222220ff));
    }
    unset_shader();
    for (int i = 0; i < LIGHT_COUNT; i++) {
        jc_draw_sphere(lights[i].position, 0.5f, colorb(lights[i].color));
    }

    if (fog_enabled) draw_fog();
    end_mode_3d();

    SDL_BlitSurfaceScaled(canvas_surface, NULL, window_surface, NULL, 0);
    SDL_UpdateWindowSurface(window);

    char buf[16];
    sprintf(buf, "%dfps", (int)(1.0/delta_time));
    SDL_SetWindowTitle(window, buf);
    return SDL_APP_CONTINUE;
}

#define MOVE_VEL 4.0
void update_player(void)
{
    Vec3 right = vec3_normalize(vec3_cross(player.look_dir, camera.up));

    // Using SDL callbacks the events pumped for us
    const bool *keys = SDL_GetKeyboardState(NULL);

    // move
    int forward_dir = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
    int strafe_dir = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
    float speed = keys[SDL_SCANCODE_LSHIFT] ? MOVE_VEL * 2 : MOVE_VEL;

    Vec3 move = { forward_dir*player.look_dir.x, 0, forward_dir*player.look_dir.z };
    move = vec3_normalize(vec3_add(move, vec3_scale(right, strafe_dir)));
    move = vec3_scale(move, speed*delta_time);

    player.pos = vec3_add(player.pos, move);
    if (player.flying) {
        if (keys[SDL_SCANCODE_SPACE]) {
            player.pos.y += speed*delta_time;
        } else if (keys[SDL_SCANCODE_LCTRL]) {
            player.pos.y -= speed*delta_time;
        }
        player.vel.y = 0.0f;
    } else {
        if (player.pos.y > 0) {
            player.vel.y -= GRAVITY * delta_time;
        } else {
            player.vel.y = 0.0f;
            if (keys[SDL_SCANCODE_SPACE]) {
                player.vel.y = 5.0f;
            }
        }
    }
    player.pos = vec3_add(player.pos, vec3_scale(player.vel, delta_time));

    // look
    float dx, dy;
    // This will be since last time we called
    SDL_GetRelativeMouseState(&dx,&dy);
    float yaw = -DEG2RAD(dx) * 0.05;
    float pitch = DEG2RAD(dy) * 0.05;
    // keyboard
    if (yaw == 0.0f) {
        int yaw_dir = keys[SDL_SCANCODE_LEFT] - keys[SDL_SCANCODE_RIGHT];
        yaw = yaw_dir * delta_time;
    }
    if (pitch == 0.0f) {
        int pitch_dir = keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP];
        pitch = pitch_dir * delta_time;
    }

    player.look_dir = vec3_transform(matrix_rotate_y(yaw), player.look_dir);
    Vec3 pitched = vec3_transform(matrix_rotate(right, pitch), player.look_dir);
    float angle = RAD2DEG(vec3_angle(camera.up, pitched));
    if (angle > 10 && angle < 170) {
        player.look_dir = pitched;
    }
    player.look_dir = vec3_normalize(player.look_dir);
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
            case SDLK_C:
                player.flying = !player.flying;
                break;
            case SDLK_F:
                fog_enabled = !fog_enabled;
                break;
            case SDLK_P:
                // print out state information
                SDL_Log("Player:");
                SDL_Log("\tpos  = { %.2f, %.2f, %.2f }", player.pos.x, player.pos.y, player.pos.z);
                SDL_Log("\tlook = { %.2f, %.2f, %.2f }", player.look_dir.x, player.look_dir.y, player.look_dir.z);
                break;
        }
    }

    return SDL_APP_CONTINUE;
}

Model create_floor(const char *path)
{
    Model model = {0};
    Vertex *verts = malloc(6*sizeof(Vertex));
    float width = 25.0f, length = 20.0f;
    float u = 0.1f*width, v = 0.1f*length;
    Vec4 c = colorf(WHITE);
    Vec3 n = { 0, 1, 0 };
    verts[0] = (Vertex){ { -1, 0, -1 }, { .x = 0, .y = v }, n, c };
    verts[1] = (Vertex){ {  1, 0,  1 }, { .x = u, .y = 0 }, n, c };
    verts[2] = (Vertex){ {  1, 0, -1 }, { .x = u, .y = v }, n, c };
    verts[3] = (Vertex){ { -1, 0, -1 }, { .x = 0, .y = v }, n, c };
    verts[4] = (Vertex){ { -1, 0,  1 }, { .x = 0, .y = 0 }, n, c };
    verts[5] = (Vertex){ {  1, 0,  1 }, { .x = u, .y = 0 }, n, c };
    
    model.vertices = verts;
    model.vertex_count = 6;
    model.transform = matrix_scale(width, 1, length);
    load_ppm(&model.texture, path);
    return model;
}

SDL_AppResult SDL_AppInit(void **state, int argc, char *argv[])
{
    (void)state, (void)argc, (void)argv;

    window = SDL_CreateWindow("jcanvas", WINX, WINY, 0);
    if (window == NULL) return SDL_APP_FAILURE;
    window_surface = SDL_GetWindowSurface(window);
    SDL_SetWindowRelativeMouseMode(window, true);

    canvas_create(&canvas, RESX, RESY);
    zbuffer = calloc(canvas.width*canvas.height, sizeof(float));
    canvas_surface = SDL_CreateSurfaceFrom(canvas.width, canvas.height, SDL_PIXELFORMAT_RGBA32,
            canvas.pixels, canvas.width*sizeof(Color));

    model_load(&diablo, "res/diablo3.obj");
    load_ppm(&diablo.texture, "res/diablo3_diffuse.ppm");

    model_load(&cannon, "res/cannon.obj");
    load_ppm(&cannon.texture, "res/cannon_diffuse.ppm");

    model_load(&building, "res/building.obj");
    load_ppm(&building.texture, "res/building.ppm");

    floor_model = create_floor("res/floor.ppm");

    // Player
    player.pos = (Vec3){ -18.28, 0.0f, 2.81 };
    player.pos = (Vec3){ -3.26, -0.06, 4.64 };
    player.look_dir = vec3_scale(vec3_normalize(player.pos), -1);
    camera.up = (Vec3){ 0, 1, 0 };
    camera.fov = DEG2RAD(60);
    camera.projection = PERSPECTIVE;


    // stuff
    float radius = 3.0f;
    for (int i = 0; i < DIABLO_COUNT; i++) {
        float ang = i * 2*M_PI / DIABLO_COUNT;
        float x = radius*SDL_cosf(ang);
        float z = radius*SDL_sin(ang);
        diablo_positions[i] = (Vec3){ x, 1, z };
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    canvas_destroy(&canvas);
    model_destroy(&diablo);
    model_destroy(&cannon);
}
