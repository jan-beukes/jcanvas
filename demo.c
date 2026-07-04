#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include "stb_image.h"

#ifndef _WIN32
#include <sys/ioctl.h>
#include <signal.h>
#endif

#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#define FRAME_TIME (1.0 / 60.0)
#define GRAVITY 10.0f
#define PLAYER_HEIGHT 1.4f

#define WINX 1280
#define WINY 720

// #define RESX WINX
// #define RESY WINY
#define RESX 320
#define RESY 180

#define LIGHT_COUNT 3

void update_player(void);

// STATE
SDL_Window *window;
SDL_Gamepad *gamepad;
SDL_Renderer *renderer;
SDL_Texture *canvas_texture;

float *zbuffer;
Canvas canvas;
Canvas frame;
int resx, resy;

Color white_pixel[1] = { WHITE };

Model sponza, diablo;

bool fog_enabled = false;
bool shading_enabled = true;
bool magnify = false;
bool terminal = false;
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
    float linear;
    float quadratic;
    Vec4 color;
} PointLight;

#define AMBIENT (Vec4){ 0.12f, 0.1f, 0.1f, 1.0f }
// https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
PointLight lights[LIGHT_COUNT] = {
    { { -30, 5, 0 }, 2.5f, 0.02f, 0.032f, { 0.5f, 0.5f, 1.00f, 1.0f } },
    { { 0, 10, 0 },  1.0f,  0.02f, 0.005f, { 1.0f, 1.0f, 1.0f, 1.0f } },
    { { 30, 5, 0 },  2.5f, 0.02f, 0.032f, { 1.0f, 0.5, 0.5f, 1.0f } },

};

bool shader(Vec4 *out, Vertex in, Vec3 pos, void *uniforms)
{
    Image *maps = uniforms;
    // sample texture
    Vec4 img_color = in.color;
    if (maps != NULL && maps[MAP_DIFFUSE].pixels != NULL) {
        img_color = image_sample(maps[MAP_DIFFUSE], in.texcoord.x, in.texcoord.y);
    }
    if (img_color.w < 0.1f) return false;

    // lighting
    Vec3 normal = vec3_normalize(in.normal);
    Vec4 color = vec4_mul(AMBIENT, img_color);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        PointLight light = lights[i];
        Vec3 light_vec = vec3_sub(light.position, pos);
        float dist = vec3_length(light_vec);
        float dot = vec3_dot(normal, vec3_normalize(light_vec));
        dot = MAX(dot, 0);
        float attenuation = 1.0f / (1 + light.linear * dist + light.quadratic*dist*dist);    
        float strength = light.strength * attenuation;
        Vec4 l = vec4_scale(light.color, dot * strength);
        Vec4 diffuse = vec4_mul(img_color, l);

        color = vec4_add(color, diffuse);
    }
    color.w = img_color.w;
    JC_Vec4 out_color = vec4_mul(in.color, color);

    *out = out_color;
    return true;
}

void draw_fog(void)
{
    Color fog_color = DARKGRAY;
    for (int y = 0; y < frame.height; y++) {
        for (int x = 0; x < frame.width; x++) {
            int depth_y = frame.height - 1 - y;

            float depth = 1.0f - zbuffer[depth_y*frame.width + x];
            float near = JC_NEAR_PLANE, far = 10;
            depth = (2.0 * near) / (far + near - depth * (far - near));	
            depth = 1.0f - expf(-2*depth*depth);
            Color p = JC_PIXEL(frame, x, y);
            Color output = color_lerp(p, fog_color, depth);
            JC_PIXEL(frame, x, y) = output;
        }
    }
}

void terminal_render(Canvas canvas)
{
    printf("\033[H");
    for (int y = 0; y < canvas.height; ++y) {
        for (int x = 0; x < canvas.width; ++x) {
            Color col = JC_PIXEL(canvas, x, y);
            printf("\033[48;2;%d;%d;%dm  ", col.r, col.g, col.b);
        }
        printf("\n");
    }
    printf("\033[m]");
}

void apply_magnification(Canvas out, Canvas in)
{
    Vec2 center = { resx/2.0f, resy/2.0f };
    float power = 1.0f;
    float radius = 50.0f;

    for (int y = 0; y < resy; y++) {
        for (int x = 0; x < resx; x++) {
            // scale by aspect ratio
            Vec2 uv = { x, y };
            float dist = vec2_length(vec2_sub(uv, center));
            if (dist > radius) continue;

            float warp = 1.0f - powf(dist / radius, power);
            warp = MAX(0, MIN(warp, 1));
            Vec2 delta = vec2_sub(center, uv);
            uv = vec2_add(uv, vec2_scale(delta, warp));
            JC_PIXEL(out, x, y) = JC_PIXEL(in, (int)uv.x, (int)uv.y);
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
    rotation += 0.4f*delta_time;

    // Rendering
    camera.position = vec3_add(player.pos, (Vec3){ 0, PLAYER_HEIGHT, 0 });
    camera.target = vec3_add(camera.position, player.look_dir);
    begin_mode_3d(frame, camera, zbuffer);

    fill(frame, colorb(AMBIENT)); 
    if (shading_enabled) set_shader(shader, NULL);
    for (int i = 0; i < sponza.mesh_count; i++) {
        Mesh mesh = sponza.meshes[i];
        set_shader_uniforms(mesh.maps);
        draw_mesh(mesh, sponza.transform, WHITE);
        // draw_mesh_wires(mesh, sponza.transform, GREEN);
    }

    set_shader_uniforms(&diablo.meshes[0].maps);
    for (int i = 0; i < LIGHT_COUNT; i++) {
        Vec3 light_xz = { lights[i].position.x, 0, lights[i].position.z };
        float radius = 3;
        float x = radius*SDL_cosf(rotation);
        float z = radius*SDL_sinf(rotation);
        diablo.transform = matrix_rotate_y(-(rotation + M_PI/2));
        diablo.transform = matrix_mul(matrix_translate(x, 1, z), diablo.transform);
        draw_model(diablo, light_xz, WHITE);
        // draw_model_wires(diablo, light_xz, GREEN);
    }

    // LIGHTS
    set_shader_uniforms(NULL);
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

    // pos processing
    memcpy(canvas.pixels, frame.pixels, frame.stride*frame.height*sizeof(Color));
    if (magnify) {
        apply_magnification(canvas, frame);
    }

    // Draw canvas to screen
    if (!terminal) {
        SDL_UpdateTexture(canvas_texture, NULL, canvas.pixels, canvas.stride*sizeof(*canvas.pixels));
        SDL_RenderTexture(renderer, canvas_texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        char buf[16];
        sprintf(buf, "%dfps", (int)(1.0/delta_time));
        SDL_SetWindowTitle(window, buf);
    } else {
        terminal_render(canvas);
    }
    return SDL_APP_CONTINUE;
}

#define MOVE_VEL 4.0
#define DEADZONE 0.1f
void update_player(void)
{
    Vec3 right = vec3_normalize(vec3_cross(player.look_dir, camera.up));

    // Using SDL callbacks the events pumped for us
    const bool *keys = SDL_GetKeyboardState(NULL);
    // move
    float forward = 0.0f, strafe = 0.0f;
    float speed = MOVE_VEL;
    if (gamepad) {
        forward = -(float)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / SDL_MAX_SINT16;
        strafe = (float)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / SDL_MAX_SINT16;
        forward = ABS(forward) < DEADZONE ? 0.0f : forward;
        strafe = ABS(strafe) < DEADZONE ? 0.0f : strafe;
        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK))
            speed *= 2;
    } else {
        forward = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
        strafe = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
        if (keys[SDL_SCANCODE_LSHIFT]) speed *= 2;
    }
    if (player.flying) speed *= 2;

    Vec3 move = { forward*player.look_dir.x, 0, forward*player.look_dir.z };
    move = vec3_normalize(vec3_add(move, vec3_scale(right, strafe)));
    move = vec3_scale(move, speed*delta_time);
    player.pos = vec3_add(player.pos, move);
    if (player.flying) {
        if (keys[SDL_SCANCODE_SPACE] || SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH)) {
            player.pos.y += speed*delta_time;
        } else if (keys[SDL_SCANCODE_LCTRL] || SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST)) {
            player.pos.y -= speed*delta_time;
        }
        player.vel.y = 0.0f;
    } else {
        if (player.pos.y > 0) {
            player.vel.y -= GRAVITY * delta_time;
        } else {
            player.vel.y = 0.0f;
            if (keys[SDL_SCANCODE_SPACE] || SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH)) {
                player.vel.y = 5.0f;
            }
        }
    }
    player.pos = vec3_add(player.pos, vec3_scale(player.vel, delta_time));

    // look
    float yaw = 0.0f, pitch = 0.0f;
    if (gamepad) {
        yaw = -(float)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / SDL_MAX_SINT16;
        pitch = (float)SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / SDL_MAX_SINT16;
        yaw = ABS(yaw) < DEADZONE ? 0.0f : yaw;
        pitch = ABS(pitch) < DEADZONE ? 0.0f : pitch;

        yaw = yaw * delta_time * 2;
        pitch = pitch * delta_time * 2;
    } else {
        float dx, dy;
        // This will be since last time we called
        SDL_GetRelativeMouseState(&dx,&dy);
        yaw = -DEG2RAD(dx) * 0.05;
        pitch = DEG2RAD(dy) * 0.05;
        // keyboard
        if (yaw == 0.0f) {
            int yaw_dir = keys[SDL_SCANCODE_LEFT] - keys[SDL_SCANCODE_RIGHT];
            yaw = yaw_dir * delta_time;
        }
        if (pitch == 0.0f) {
            int pitch_dir = keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP];
            pitch = pitch_dir * delta_time;
        }
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
            case SDLK_L:
                shading_enabled = !shading_enabled;
                break;
            case SDLK_M:
                magnify = !magnify;
                break;
            default:
                break;
        }
    } else if (e->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (e->gbutton.button) {
            case SDL_GAMEPAD_BUTTON_WEST:
                player.flying = !player.flying;
                break;
            case SDL_GAMEPAD_BUTTON_NORTH:
                shading_enabled = !shading_enabled;
                break;
            default:
                break;
        }
    } else if (e->type == SDL_EVENT_MOUSE_WHEEL) {
        camera.fov -= DEG2RAD(e->wheel.y);
        camera.fov = MAX(DEG2RAD(5), camera.fov);
        camera.fov = MIN(DEG2RAD(120), camera.fov);
    } else if (e->type == SDL_EVENT_GAMEPAD_ADDED) {
        if (gamepad == NULL) {
            gamepad = SDL_OpenGamepad(e->gdevice.which);
        }
    } else if (e->type == SDL_EVENT_GAMEPAD_REMOVED) {
        if (gamepad && (SDL_GetGamepadID(gamepad) == e->gdevice.which)) {
            SDL_CloseGamepad(gamepad);
            gamepad = NULL;
        }
    }

    return SDL_APP_CONTINUE;
}

void reset_and_exit(int sig)
{
    (void)sig;
    // show cursor
    printf("\033[?25h");
    printf("\033[2J");
    exit(0);
}

SDL_AppResult SDL_AppInit(void **state, int argc, char *argv[])
{
    (void)state, (void)argc, (void)argv;

    SDL_InitFlags init_flags = SDL_INIT_GAMEPAD;
    if (argc > 1 && SDL_strcmp(argv[1], "term") == 0) {
        terminal = true;
        init_flags |= SDL_INIT_VIDEO;
    }
    SDL_Init(init_flags);

    if (terminal) {
#ifdef _WIN32
        resx = 45;
        resy = 25;
#else
        struct winsize size;
        ioctl(1, TIOCGWINSZ, &size);
        resy = size.ws_row - 1;
        resx = resy * 4 / 3;

        signal(SIGINT, reset_and_exit);
        // Hide cursor and clear screen
        printf("\033[?25l");
        printf("\033[2J");
#endif
    } else {
        resx = RESX, resy = RESY;
        window = SDL_CreateWindow("jcanvas", WINX, WINY, SDL_WINDOW_RESIZABLE);
        if (window == NULL) return SDL_APP_FAILURE;
        renderer = SDL_CreateRenderer(window, NULL);
        if (renderer == NULL) return SDL_APP_FAILURE;
        SDL_SetWindowRelativeMouseMode(window, true);
        float aspect = (float)WINX/WINY;
        SDL_SetWindowAspectRatio(window, aspect, aspect);

        canvas_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, resx, resy);
        SDL_SetTextureScaleMode(canvas_texture, SDL_SCALEMODE_NEAREST);
    }

    // screen canvas
    canvas_create(&canvas, resx, resy);
    // da frame
    canvas_create(&frame, resx, resy);
    zbuffer = calloc(canvas.width*canvas.height, sizeof(float));

    model_load(&diablo, "res/diablo3.obj");

    model_load(&sponza, "res/sponza/sponza.obj");
    sponza.transform = matrix_scale(0.6, 0.6, 0.6);
    sponza.transform = matrix_mul(matrix_translate(0, 0, 1), sponza.transform);
    // for (int i = 0; i < sponza.mesh_count; i++) {
    //     sponza.meshes[i].maps[0].flags |= FILTER_BILINEAR;
    // }

    // Player
    player.pos = (Vec3){ -18.28, 0.0f, 2.81 };
    player.pos = (Vec3){ -3.26, -0.06, 4.64 };
    player.look_dir = vec3_scale(vec3_normalize(player.pos), -1);
    camera.up = (Vec3){ 0, 1, 0 };
    camera.fov = DEG2RAD(60);
    camera.projection = PERSPECTIVE;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result)
{
    (void)result, (void)state;
    canvas_destroy(&canvas);
    model_destroy(&diablo);

    if (terminal) {
        // show cursor
        printf("\033[?25h");
        printf("\033[2J");
    }
}
