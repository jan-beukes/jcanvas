// This is an STB style single header library.
// One of your source files must define JCANVAS_IMPLEMENTATION before this header is included.
// This will include all of the implementation into that source file
// Otherwise the header can be included as normal everywhere else in your program

// There is also currently minimal namespacing used with functions and types
// TODO: add prefixes and strip them by default with an option to not strip prefixes similar to what
// is done in https://github.com/tsoding/nob.h
#ifndef JCANVAS_H
#define JCANVAS_H

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

// This is in RGBA for when alpha blending is added
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

// NOTE: This is taken from raylib https://github.com/raysan5/raylib
// MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized with { }
// This is called aggregate initialization (C++11 feature)
#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif
#define LIGHTGRAY  CLITERAL(Color){ 200, 200, 200, 255 }
#define GRAY       CLITERAL(Color){ 130, 130, 130, 255 }
#define DARKGRAY   CLITERAL(Color){ 80, 80, 80, 255 }
#define YELLOW     CLITERAL(Color){ 253, 249, 0, 255 }
#define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }
#define ORANGE     CLITERAL(Color){ 255, 161, 0, 255 }
#define PINK       CLITERAL(Color){ 255, 109, 194, 255 }
#define RED        CLITERAL(Color){ 230, 41, 55, 255 }
#define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }
#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 }
#define LIME       CLITERAL(Color){ 0, 158, 47, 255 }
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 }
#define SKYBLUE    CLITERAL(Color){ 102, 191, 255, 255 }
#define BLUE       CLITERAL(Color){ 0, 121, 241, 255 }
#define DARKBLUE   CLITERAL(Color){ 0, 82, 172, 255 }
#define PURPLE     CLITERAL(Color){ 200, 122, 255, 255 }
#define VIOLET     CLITERAL(Color){ 135, 60, 190, 255 }
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }
#define BEIGE      CLITERAL(Color){ 211, 176, 131, 255 }
#define BROWN      CLITERAL(Color){ 127, 106, 79, 255 }
#define DARKBROWN  CLITERAL(Color){ 76, 63, 47, 255 }
#define WHITE      CLITERAL(Color){ 255, 255, 255, 255 }
#define BLACK      CLITERAL(Color){ 0, 0, 0, 255 }
#define BLANK      CLITERAL(Color){ 0, 0, 0, 0 }
#define MAGENTA    CLITERAL(Color){ 255, 0, 255, 255 }

// The main canvas/image structure
typedef struct {
    int width, height;
    Color *pixels;
} Canvas;

#define CV_PIXEL(c, x, y) (c).pixels[y*(c).width + x]

// Mr C++ don't mangle my function names
#if defined(__cplusplus)
extern "C" {
#endif

bool cv_create(Canvas *canvas, int width, int height);
void cv_destroy(Canvas *canvas);
bool cv_load_ppm(Canvas *canvas, const char *path);
bool cv_save_ppm(Canvas canvas, const char *path);

// clamps x and y to the canvas
void cv_clamp(Canvas canvas, int *x, int *y);

// These functions are used to draw another canvas/image to a canvas
void cv_blit(Canvas canvas, Canvas image, int x, int y);
// Like cv_blit but it scales the image so that it occupies the rectangle (x, y, w, h)
void cv_blit_rect(Canvas canvas, Canvas image, int x, int y, int w, int h);

void cv_rect(Canvas canvas, int x, int y, int w, int h, Color color);
void cv_fill(Canvas canvas, Color color);
void cv_line(Canvas canvas, int x1, int y1, int x2, int y2, Color color);

// converts a color stored in an integer to a color struct
// 0xaa00aaff will be a purple color as the LSB end is assumed stores alpha
Color color_from_int(uint32_t color);
Color color_blend_alpha(Color a, Color b);

#if defined(__cplusplus)
}
#endif

#ifdef JCANVAS_IMPLEMENTATION
//======================================
// IMPLEMENTATION
//======================================

#define _MIN(x, y) ((x) < (y) ? (x) : (y))
#define _MAX(x, y) ((x) > (y) ? (x) : (y))

bool cv_create(Canvas *canvas, int width, int height)
{
    canvas->width = width;
    canvas->height = height;
    size_t size = width * height * sizeof(Color);
    canvas->pixels = malloc(size);
    if (canvas->pixels == NULL) return false;
    memset(canvas->pixels, 0, size);
    return true;
}

void cv_destroy(Canvas *canvas)
{
    free(canvas->pixels);
    canvas->pixels = NULL;
}

// load a ppm file
bool cv_load_ppm(Canvas *canvas, const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return false;
    }

    long size;
    if (fseek(file, 0, SEEK_END) < 0) return false;
    if ((size = ftell(file)) < 0) return false;
    if (fseek(file, 0, SEEK_SET) < 0) return false;
    char *buf = malloc(size);
    if (buf == NULL) return false;
    fread(buf, size, 1, file);

    fclose(file);

    char *b = buf;
    char header[2] = { *b++, *b++ };
    if (strcmp(header, "P6") != 0) return false;
    // skip space;
    while (isspace(*b)) b++;

    char *endptr;
    int width, height, maxval;
    // width
    width = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (isspace(*b)) b++;

    // height
    height = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (isspace(*b)) b++;

    // maxval
    maxval = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (isspace(*b)) b++;
    assert(maxval == 255);

    canvas->width = width;
    canvas->height = height;
    canvas->pixels = malloc(width*height*sizeof(Color));

    // parse pixels
    assert(size - (b - buf) == 3*width*height);
    uint8_t *image_pixels = (uint8_t*)b;
    for (int i = 0; i < width*height; ++i) {
        Color c = {
            .r = image_pixels[3*i],
            .g = image_pixels[3*i + 1],
            .b = image_pixels[3*i + 2],
            .a = 255,
        };
        canvas->pixels[i] = c;
    }
    return true;
}

bool cv_save_ppm(Canvas canvas, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return false;
    }
    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n%d\n", canvas.width, canvas.height, 255);
    for (int i = 0; i < canvas.width*canvas.height; i++) {
        Color color = canvas.pixels[i];
        uint8_t c[3] = { color.r, color.g, color.b };
        fwrite(c, sizeof(c), 1, f);
    }

    fclose(f);
    return true;
}

void cv_clamp(Canvas canvas, int *x, int *y)
{
    int x_clamped = _MAX(*x, 0);
    *x = _MIN(x_clamped, canvas.width);
    int y_clamped = _MAX(*y, 0);
    *y = _MIN(y_clamped, canvas.height);
}

void cv_fill(Canvas canvas, Color color)
{
    for (int y = 0; y < canvas.height; ++y) {
        for (int x = 0; x < canvas.width; ++x) {
            CV_PIXEL(canvas, x, y) = color;
        }
    }
}

// TODO: look into different ways of alpha blending
void cv_blit(Canvas canvas, Canvas image, int x, int y)
{
    if (x == 0 && y == 0 && canvas.width == image.width && canvas.height == image.height) {
        memcpy(canvas.pixels, image.pixels, canvas.width*canvas.height*sizeof(Color));
        return;
    }

    int x_max = x + image.width;
    int y_max = y + image.width;
    cv_clamp(canvas, &x, &y);
    cv_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++y) {
        for (int j = x; x < x_max; ++x) {
            Color image_pixel = CV_PIXEL(image, j - x, i - y);
            Color canvas_pixel = CV_PIXEL(canvas, j, i);
            CV_PIXEL(canvas, j, i) = color_blend_alpha(canvas_pixel, image_pixel);
        }
    }
}

// TODO: Add interpolation option
void cv_blit_rect(Canvas canvas, Canvas image, int x, int y, int w, int h)
{
    if (w == (int)image.width && h == (int)image.height) {
        cv_blit(canvas, image, x, y);
        return;
    }

    int x_max = x + w;
    int y_max = y + h;
    cv_clamp(canvas, &x, &y);
    cv_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++i) {
        for (int j = x; j < x_max; ++j) {
            int image_x = (j - x) * image.width / w;
            int image_y = (i - y) * image.height / h;

            Color image_pixel = CV_PIXEL(image, image_x, image_y);
            Color canvas_pixel = CV_PIXEL(canvas, j, i);
            CV_PIXEL(canvas, j, i) = color_blend_alpha(canvas_pixel, image_pixel);
        }
    }
}

void cv_rect(Canvas canvas, int x, int y, int w, int h, Color color)
{
    int x_max = x + w;
    int y_max = y + h;
    cv_clamp(canvas, &x, &y);
    cv_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++i) {
        for (int j = x; j < x_max; ++j) {
            CV_PIXEL(canvas, j, i) = color;
        }
    }
}

void cv_line(Canvas canvas, int x1, int y1, int x2, int y2, Color color)
{
    for (float t = 0.0f; t < 1.0f; t += 0.02f) {
        int x = roundf(x1 + t * (x2 - x1));
        int y = roundf(y1 + t * (y2 - y1));
        CV_PIXEL(canvas, x, y) = color;
    }
}

// Blend b onto a
Color color_blend_alpha(Color ca, Color cb)
{
    uint32_t alpha = cb.a;
    uint32_t r = (ca.r*(255 - alpha) + cb.r*alpha)/255;
    uint32_t g = (ca.g*(255 - alpha) + cb.g*alpha)/255;
    uint32_t b = (ca.b*(255 - alpha) + cb.b*alpha)/255;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    Color c = { r, g, b, ca.a };
    return c;
}

Color color_from_int(uint32_t color)
{
    Color c = {
        .r = (color >> 24) & 0xff,
        .g = (color >> 16) & 0xff,
        .b = (color >> 8)  & 0xff,
        .a = (color >> 0)  & 0xff,
    };
    return c;
}

#endif // JCANVAS_IMPLEMENTATION
//======================================

#endif // jCANVAS_H
