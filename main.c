#define RGFW_IMPLEMENTATION
#include "RGFW.h"

#define JCANVAS_IMPLEMENTATION
#include "jcanvas.h"

#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define RESX 64
#define RESY 64

Canvas image;

void draw(Canvas canvas)
{
    cv_fill(canvas, BLACK);
    cv_blit_rect(canvas, image, 0, 0, canvas.width, canvas.height);

    int ax = 7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    cv_line(canvas, ax, ay, bx, by, BLUE);
    cv_line(canvas, cx, cy, bx, by, GREEN);
    cv_line(canvas, cx, cy, ax, ay, YELLOW);
    cv_line(canvas, ax, ay, cx, cy, RED);
}

int main(void)
{
    RGFW_init();
    RGFW_monitor *monitor = RGFW_getPrimaryMonitor();
    const int win_width = 8*RESX;
    const int win_height = 8*RESY;
    RGFW_window *window = RGFW_createWindow("Render", monitor->x, monitor->y,
           win_width, win_height, RGFW_windowCenter);
    RGFW_window_setExitKey(window, RGFW_keyEscape);

    Canvas canvas, win_canvas;
    cv_create(&canvas, RESX, RESY);
    cv_create(&win_canvas, win_width, win_height);

    cv_load_ppm(&image, "image.ppm");

    RGFW_surface *surface = RGFW_createSurface((uint8_t*)win_canvas.pixels,
            win_canvas.width, win_canvas.height, RGFW_formatRGBA8);

    while (!RGFW_window_shouldClose(window)) {
        RGFW_pollEvents();
        draw(canvas);
        // scale to window
        cv_blit_rect(win_canvas, canvas, 0, 0, win_canvas.width, win_canvas.height);

        RGFW_window_blitSurface(window, surface);
    }
    cv_save_ppm(canvas, "slop.ppm");

    cv_destroy(&canvas);
    cv_destroy(&image);
    cv_destroy(&win_canvas);
    RGFW_surface_free(surface);
    RGFW_window_close(window);
}
