#define RGFW_IMPLEMENTATION
#include "RGFW.h"

#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define RESX 600
#define RESY 600

JC_Canvas canvas;
JC_Model model;

float theta = 0;
float x = 0;
void draw(void)
{
    theta += 0.02f;
    x += 0.01;
    model.transform = jc_matrix_rotatey(theta);
    model.transform = jc_matrix_mul(model.transform, jc_matrix_translate(x, 0, 0));
    jc_fill(canvas, BLACK);
    jc_model(canvas, model, RED);
    for (int i = 0; i < model.vertex_count; i++) {
        JC_Vec3 pos = jc_vec3_transform(model.transform, model.vertices[i].position);
        JC_Vec2 p = jc_canvas_coord(canvas, pos);
        jc_pixel(canvas, p.x, p.y, WHITE);
    }
}

void init(void)
{
    jc_create(&canvas, RESX, RESY);
    jc_load_obj(&model, "res/diablo3.obj");
    model.transform = jc_matrix_translate(0, 0, 1);
}

int main(void)
{
    RGFW_init();
    RGFW_monitor *monitor = RGFW_getPrimaryMonitor();
    const int win_width = RESX;
    const int win_height = RESY;
    // TODO: is there any way to fix the flickering on resize when working with window surfaces on X11
    RGFW_window *window = RGFW_createWindow("Render", monitor->x, monitor->y,
           win_width, win_height, RGFW_windowNoResize|RGFW_windowCenter);
    RGFW_window_setExitKey(window, RGFW_keyEscape);

    init();
    JC_Canvas win_canvas;
    jc_create(&win_canvas, win_width, win_height);
    RGFW_surface *surface = RGFW_createSurface((uint8_t*)win_canvas.pixels,
            win_canvas.width, win_canvas.height, RGFW_formatRGBA8);

    double frame_time = 1.0 / 60.0;
    double prev_frame = 0.0;
    while (!RGFW_window_shouldClose(window)) {
        double now = clock() / (double)CLOCKS_PER_SEC;
        if (now - prev_frame > frame_time) {
            prev_frame = now;
            RGFW_pollEvents();

            draw();
            // scale to window
            jc_blit_rect(win_canvas, canvas, 0, 0, win_canvas.width, win_canvas.height);
            RGFW_window_blitSurface(window, surface);
        }
    }

    jc_destroy(&canvas);
    jc_destroy(&win_canvas);
    RGFW_surface_free(surface);
    RGFW_window_close(window);
    return 0;
}
