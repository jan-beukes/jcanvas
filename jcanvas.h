// jcanvas is a simple software rendering library for drawing into a pixel buffer
// Most of the rendering techniques were learned from the amazing series https://haqr.eu/tinyrenderer
// Raylib source code was also used as a reference for many different things
//
// This is an STB style single header library.
// One of your source files must define JCANVAS_IMPLEMENTATION before this header is included.
// This will include all of the implementation into that source file
// Otherwise the header can be included as normal everywhere else in your program
//
// By default the prefix is stripped from the types and most functions. See the bottom of this file for more information.
// The prefixes can be enabled by defining JC_PREFIX before you include this file. This is needed

#ifndef JCANVAS_H
#define JCANVAS_H

// TODO: Customizable math functions, alloc functions and maybe assert?
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

// This is in RGBA for when alpha blending is added
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} JC_Color;

// NOTE: This is taken from raylib https://github.com/raysan5/raylib
#if defined(__cplusplus)
    #define CLITERAL(type) type
#else
    #define CLITERAL(type) (type)
#endif
#define LIGHTGRAY  CLITERAL(JC_Color){ 200, 200, 200, 255 }
#define GRAY       CLITERAL(JC_Color){ 130, 130, 130, 255 }
#define DARKGRAY   CLITERAL(JC_Color){ 80, 80, 80, 255 }
#define YELLOW     CLITERAL(JC_Color){ 253, 249, 0, 255 }
#define GOLD       CLITERAL(JC_Color){ 255, 203, 0, 255 }
#define ORANGE     CLITERAL(JC_Color){ 255, 161, 0, 255 }
#define PINK       CLITERAL(JC_Color){ 255, 109, 194, 255 }
#define RED        CLITERAL(JC_Color){ 230, 41, 55, 255 }
#define MAROON     CLITERAL(JC_Color){ 190, 33, 55, 255 }
#define GREEN      CLITERAL(JC_Color){ 0, 228, 48, 255 }
#define LIME       CLITERAL(JC_Color){ 0, 158, 47, 255 }
#define DARKGREEN  CLITERAL(JC_Color){ 0, 117, 44, 255 }
#define SKYBLUE    CLITERAL(JC_Color){ 102, 191, 255, 255 }
#define BLUE       CLITERAL(JC_Color){ 0, 121, 241, 255 }
#define DARKBLUE   CLITERAL(JC_Color){ 0, 82, 172, 255 }
#define PURPLE     CLITERAL(JC_Color){ 200, 122, 255, 255 }
#define VIOLET     CLITERAL(JC_Color){ 135, 60, 190, 255 }
#define DARKPURPLE CLITERAL(JC_Color){ 112, 31, 126, 255 }
#define BEIGE      CLITERAL(JC_Color){ 211, 176, 131, 255 }
#define BROWN      CLITERAL(JC_Color){ 127, 106, 79, 255 }
#define DARKBROWN  CLITERAL(JC_Color){ 76, 63, 47, 255 }
#define WHITE      CLITERAL(JC_Color){ 255, 255, 255, 255 }
#define BLACK      CLITERAL(JC_Color){ 0, 0, 0, 255 }
#define BLANK      CLITERAL(JC_Color){ 0, 0, 0, 0 }
#define MAGENTA    CLITERAL(JC_Color){ 255, 0, 255, 255 }

// The main canvas/image structure in RGBA8 format
// TODO: pitch for subcanvas ?
typedef struct {
    int width, height;
    JC_Color *pixels;
} JC_Canvas;

// This typedef is just a way to document usage since these are the exact same structure
typedef JC_Canvas JC_Image;

typedef struct {
    float x, y;
} JC_Vec2;

typedef struct {
    float x, y, z;
} JC_Vec3;

typedef struct {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} JC_Matrix;

typedef struct {
    JC_Vec3 position;
    JC_Vec3 texcoord;
    JC_Vec3 normal;
    JC_Color color;
} JC_Vertex;

// TODO: make da camera
typedef struct {
    JC_Vec3 position;
} JC_Camera;

// The faces are currently only triangles
typedef struct {
    JC_Vertex *vertices;
    int vertex_count;
    JC_Matrix transform;

    // This is what will be applied when no shader func is set
    // If you want to do something more advanced set the shader func
    // and pass uniforms to sample textures
    JC_Image texture;
} JC_Model;

// NOTE: Do not try to mutate uniforms in this function since this can be called by seperate threads when OpenMP is used.
// This is a "shader" that is called for every pixel during rasterization
// input will hold the interpolated values of the vertices given to the rasterize function
// If no texture is given to the rasterize function it will be NULL
typedef JC_Color (*JC_ShaderFunc)(JC_Vertex input, JC_Image *texture, void *uniforms);

#define JC_PIXEL(c, x, y) (c).pixels[y*(c).width + x]
#define JC_IN_BOUNDS(c, x, y) (x >= 0 && x < (c).width && y >= 0 && y < (c).height)

// Mr C++ don't mangle my function names
#if defined(__cplusplus)
extern "C" {
#endif

bool jc_create(JC_Canvas *canvas, int width, int height);
void jc_destroy(JC_Canvas *canvas);
void jc_resize(JC_Canvas *canvas, int w, int h);

// These are currently the only image loading functions provided
// If you want to load an image then you can simply load the data using something like stb_image
// and manually initialize the canvas pixels, width and height or use jc_create and fill the pixels
bool jc_load_ppm(JC_Image *image, const char *path);
bool jc_save_ppm(JC_Image image, const char *path);

// TODO: add QOI https://github.com/phoboslab/qoi/blob/master/qoi.h
// load_qoi load_qoi_from_memory and save_qoi

// This is for the obj format!
// Similarly to image loading the Model object is not very complicated and you can easily use some
// other library to load a model and then convert the vertices to JC_Vertex (or just convert to obj)
bool jc_model_load(JC_Model *model, const char *path);
bool jc_model_load_from_memory(JC_Model *model, char *data, long size);
void jc_model_destroy(JC_Model *model);

// clamps x and y to the canvas
void jc_clamp(JC_Canvas canvas, int *x, int *y);

// These functions are used to draw another canvas/image to a canvas
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y);
// Like cv_blit but it scales the image so that it occupies the rectangle (x, y, w, h)
void jc_blit_rect(JC_Canvas canvas, JC_Image image, int x, int y, int w, int h);

// TODO: vector argument functions?
void jc_fill(JC_Canvas canvas, JC_Color color);
void jc_draw_pixel(JC_Canvas canvas, int x, int y, JC_Color color);
void jc_draw_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color);
void jc_draw_line(JC_Canvas canvas, int ax, int ay, int bx, int by, JC_Color color);
void jc_draw_triangle(JC_Canvas canvas, int ax, int ay, int bx, int by, int cx, int cy, JC_Color color);

// The shader func is only called in the rasterize functions
void jc_set_shader_func(JC_ShaderFunc func, void *uniforms);
void jc_unset_shader_func(void);
void jc_rasterize_triangle(JC_Canvas canvas, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3);
void jc_rasterize_triangle_ex(JC_Canvas canvas, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3, JC_Image texture, JC_Color color);

void jc_draw_model(JC_Canvas canvas, JC_Model model, JC_Color color);
void jc_draw_model_wires(JC_Canvas canvas, JC_Model model, JC_Color color);

// converts a color stored in an integer to a color struct
// 0xaa00aaff will be a purple color as the LSB end is assumed stores alpha
JC_Color jc_color_from_int(uint32_t color);
JC_Color jc_color_blend_alpha(JC_Color a, JC_Color b);
JC_Color jc_color_mul(JC_Color a, JC_Color b);

//---Linear algebra functions---
JC_Vec3 jc_project(JC_Vec3 point);
JC_Vec2 jc_canvas_coord(JC_Canvas, JC_Vec3 point);

JC_Vec3 jc_vec3_add(JC_Vec3 a, JC_Vec3 b);
JC_Vec3 jc_vec3_sub(JC_Vec3 a, JC_Vec3 b);
JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v);
JC_Vec3 jc_vec3_normalize(JC_Vec3 v);
JC_Vec3 jc_vec3_cross(JC_Vec3 a, JC_Vec3 b);
float jc_vec3_dot(JC_Vec3 a, JC_Vec3 b);
float jc_vec3_length(JC_Vec3 v);

JC_Matrix jc_matrix_identity(void);

JC_Matrix jc_matrix_translate(float x, float y, float z);
JC_Matrix jc_matrix_scale(float s);
JC_Matrix jc_matrix_rotate_x(float angle);
JC_Matrix jc_matrix_rotate_y(float angle);
JC_Matrix jc_matrix_rotate_z(float angle);

JC_Matrix jc_matrix_add(JC_Matrix a, JC_Matrix b);
JC_Matrix jc_matrix_sub(JC_Matrix a, JC_Matrix b);
JC_Matrix jc_matrix_mul(JC_Matrix a, JC_Matrix b);

#if defined(__cplusplus)
}
#endif

#ifdef JCANVAS_IMPLEMENTATION
//======================================
// IMPLEMENTATION
//======================================

static JC_ShaderFunc jc_active_shader_func = NULL;
static void *jc_active_shader_uniforms = NULL;

// TODO: maybe make these part of the API?
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SWAP(x, y) do { \
    __typeof__(x) tmp = x; \
    x = y; \
    y = tmp; \
} while(0)

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define da_append(da, item) \
do { \
    if ((da)->capacity == 0) { \
        (da)->capacity = 16; \
        (da)->count = 0; \
        (da)->items = malloc((da)->capacity*sizeof((da)->items[0])); \
    } \
    if ((da)->count + 1 >= (da)->capacity) { \
        (da)->capacity *= 2; \
        (da)->items = realloc((da)->items, (da)->capacity*sizeof((da)->items[0])); \
    } \
    (da)->items[(da)->count++] = item; \
} while (0)

bool jc_create(JC_Canvas *canvas, int width, int height)
{
    canvas->width = width;
    canvas->height = height;
    size_t size = width * height * sizeof(JC_Color);
    canvas->pixels = malloc(size);
    if (canvas->pixels == NULL) return false;
    for (int i = 0; i < width*height; ++i) {
        JC_Color c = { 0, 0, 0, 255 };
        canvas->pixels[i] = c;
    }
    return true;
}

void jc_destroy(JC_Canvas *canvas)
{
    free(canvas->pixels);
    canvas->pixels = NULL;
}

void jc_resize(JC_Canvas *canvas, int w, int h)
{
    canvas->pixels = realloc(canvas->pixels, w*h*sizeof(JC_Color));
    canvas->width = w;
    canvas->height = h;
}

char *_read_entire_file(const char *path, long *size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    long file_size;
    if (fseek(file, 0, SEEK_END) < 0) return NULL;
    if ((file_size = ftell(file)) < 0) return NULL;
    if (fseek(file, 0, SEEK_SET) < 0) return NULL;
    char *buf = malloc(file_size + 1);
    if (buf == NULL) return NULL;
    fread(buf, file_size, 1, file);
    if (size) *size = file_size;
    buf[file_size-1] = '\0';

    fclose(file);
    return buf;
}

// load a ppm file
bool jc_load_ppm(JC_Canvas *image, const char *path)
{
    long size;
    char *data = _read_entire_file(path, &size);
    if (data == NULL) return false;

    char *b = data;
    if (*b++ != 'P' || *b++ != '6') return false;
    // skip space;
    while (b && isspace(*b)) b++;

    char *endptr;
    int width, height, maxval;
    // width
    width = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (b && isspace(*b)) b++;

    // height
    height = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (b && isspace(*b)) b++;

    // maxval
    maxval = strtol(b, &endptr, 0);
    if (endptr == b) return false;
    b = endptr;
    while (b && isspace(*b)) b++;
    assert(maxval == 255);

    image->width = width;
    image->height = height;
    image->pixels = malloc(width*height*sizeof(JC_Color));

    // parse pixels
    assert(size - (b - data) == 3*width*height);
    uint8_t *image_pixels = (uint8_t*)b;
    for (int i = 0; i < width*height; ++i) {
        JC_Color c = {
            .r = image_pixels[3*i],
            .g = image_pixels[3*i + 1],
            .b = image_pixels[3*i + 2],
            .a = 255,
        };
        image->pixels[i] = c;
    }
    free(data);
    return true;
}

bool jc_save_ppm(JC_Canvas image, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return false;
    }
    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n%d\n", image.width, image.height, 255);
    for (int i = 0; i < image.width*image.height; i++) {
        JC_Color color = image.pixels[i];
        uint8_t c[3] = { color.r, color.g, color.b };
        fwrite(c, sizeof(c), 1, f);
    }

    fclose(f);
    return true;
}

struct vec3_array {
    JC_Vec3 *items;
    int count;
    int capacity;
};

struct vertex_array {
    JC_Vertex *items;
    int count;
    int capacity;
};

bool jc_model_load_from_memory(JC_Model *model, char *data, long size)
{
    char *line;
    char *ptr = data;
    struct vec3_array positions = {0};
    struct vec3_array texcoords = {0};
    struct vec3_array normals = {0};
    struct vertex_array vertices = {0};
    while((line = strtok_r(ptr, "\n", &ptr))) {
        char type[16];
        sscanf(line, "%s", type);
        if (strcmp(type, "#") == 0) continue;

        char *s = strstr(line, type);
        while ((s - data < size) && !isspace(*s)) s++;
        if (strcmp(type, "v") == 0) {
            float x, y, z, w;
            int n = sscanf(s, "%f %f %f %f", &x, &y, &z, &w);
            if (n > 3) {
                x *= w;
                y *= w;
                z *= w;
            }
            JC_Vec3 pos = { x, y, z };
            da_append(&positions, pos);
        } else if (strcmp(type, "vt") == 0) {
            float u, v, w;
            sscanf(s, "%f %f %f", &u, &v, &w);
            JC_Vec3 tex = { u, v, w };
            da_append(&texcoords, tex);
        } else if (strcmp(type, "vn") == 0) {
            float x, y, z;
            sscanf(s, "%f %f %f", &x, &y, &z);
            JC_Vec3 norm = { x, y, z };
            da_append(&normals, norm);
        } else if (strcmp(type, "f") == 0) {
            int v, vt, vn;
            JC_Vertex vertex;
            vertex.color = WHITE;
            for (int i = 0; i < 3; i++) {
                char *end;
                // vertex
                v = strtol(s, &end, 0) - 1;
                vertex.position = positions.items[v];
                s = end + 1;
                if (*end != '/') {
                    da_append(&vertices, vertex);
                    continue;
                }
                // texcoord
                vt = strtol(s, &end, 0);
                vertex.texcoord = texcoords.items[vt];
                s = end + 1;
                if (*end != '/') {
                    da_append(&vertices, vertex);
                    continue;
                }
                // normal
                vn = strtol(s, &end, 0);
                vertex.normal = normals.items[vn];
                s = end + 1;
                da_append(&vertices, vertex);
            }
        } else {
            // TODO: handle other things https://en.wikipedia.org/wiki/Wavefront_.obj_file
        }
    }
    free(positions.items);
    free(texcoords.items);
    free(normals.items);

    memset(model, 0, sizeof(*model));
    model->vertex_count = vertices.count;
    model->vertices = realloc(vertices.items, model->vertex_count * sizeof(JC_Vertex));
    model->transform = jc_matrix_identity();
    return true;
}

bool jc_model_load(JC_Model *model, const char *path)
{
    long size;
    char *data = _read_entire_file(path, &size);
    if (data == NULL) return false;
    bool result = jc_model_load_from_memory(model, data, size);
    free(data);
    return result;
}

void jc_model_destroy(JC_Model *model)
{
    free(model->vertices);
    model->vertices = NULL;
    jc_destroy(&model->texture);
}

//=================
// Rendering
//=================
void jc_clamp(JC_Canvas canvas, int *x, int *y)
{
    int x_clamped = MAX(*x, 0);
    *x = MIN(x_clamped, canvas.width - 1);
    int y_clamped = MAX(*y, 0);
    *y = MIN(y_clamped, canvas.height - 1);
}

// TODO: look into different ways of blending (https://wiki.libsdl.org/SDL3/SDL_BlendMode)
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y)
{
    if (!JC_IN_BOUNDS(canvas, x, y) && !JC_IN_BOUNDS(canvas, x+image.width, y+image.height)) return;
    if (x == 0 && y == 0 && canvas.width == image.width && canvas.height == image.height) {
        memcpy(canvas.pixels, image.pixels, canvas.width*canvas.height*sizeof(JC_Color));
        return;
    }

    int x_max = x + image.width;
    int y_max = y + image.width;
    jc_clamp(canvas, &x, &y);
    jc_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++y) {
        for (int j = x; x < x_max; ++x) {
            JC_Color image_pixel = JC_PIXEL(image, j - x, i - y);
            JC_Color canvas_pixel = JC_PIXEL(canvas, j, i);
            JC_PIXEL(canvas, j, i) = jc_color_blend_alpha(canvas_pixel, image_pixel);
        }
    }
}

// TODO: Add interpolation option
void jc_blit_rect(JC_Canvas canvas, JC_Image image, int x, int y, int w, int h)
{
    if (!JC_IN_BOUNDS(canvas, x, y) && !JC_IN_BOUNDS(canvas, x+w, y+h)) return;
    if (w == (int)image.width && h == (int)image.height) {
        jc_blit(canvas, image, x, y);
        return;
    }

    int x_max = x + w;
    int y_max = y + h;
    jc_clamp(canvas, &x, &y);
    jc_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++i) {
        for (int j = x; j < x_max; ++j) {
            int image_x = (j - x) * image.width / w;
            int image_y = (i - y) * image.height / h;

            JC_Color image_pixel = JC_PIXEL(image, image_x, image_y);
            JC_Color canvas_pixel = JC_PIXEL(canvas, j, i);
            JC_PIXEL(canvas, j, i) = jc_color_blend_alpha(canvas_pixel, image_pixel);
        }
    }
}

void jc_fill(JC_Canvas canvas, JC_Color color)
{
    for (int y = 0; y < canvas.height; ++y) {
        for (int x = 0; x < canvas.width; ++x) {
            JC_PIXEL(canvas, x, y) = color;
        }
    }
}

void jc_draw_pixel(JC_Canvas canvas, int x, int y, JC_Color color)
{
    if (!JC_IN_BOUNDS(canvas, x, y)) return;
    JC_PIXEL(canvas, x, y) = color;
}

void jc_draw_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color)
{
    if (!JC_IN_BOUNDS(canvas, x, y) && !JC_IN_BOUNDS(canvas, x+w, y+h)) return;

    int x_max = x + w;
    int y_max = y + h;
    jc_clamp(canvas, &x, &y);
    jc_clamp(canvas, &x_max, &y_max);
    for (int i = y; i < y_max; ++i) {
        for (int j = x; j < x_max; ++j) {
            JC_PIXEL(canvas, j, i) = color;
        }
    }
}

void jc_draw_line(JC_Canvas canvas, int ax, int ay, int bx, int by, JC_Color color)
{
    if (!JC_IN_BOUNDS(canvas, ax, ay) && !JC_IN_BOUNDS(canvas, bx, by)) return;
    // if steep we swap x and y to iterate over y
    bool steep = ABS(ax - bx) < ABS(ay - by);
    if (steep) {
        SWAP(ax, ay);
        SWAP(bx, by);
    }
    // make sure that ax is the smallest of the points
    if (ax > bx) {
        SWAP(ax, bx);
        SWAP(ay, by);
    }

    jc_clamp(canvas, &ax, &ay);
    jc_clamp(canvas, &bx, &by);
    float y = ay;
    float dy = (float)(by-ay) / (bx-ax);
    for (int x = ax; x <= bx; ++x) {
        // since y is actually x in this case
        if (steep) {
            JC_PIXEL(canvas, (int)y, x) = color;
        } else {
            JC_PIXEL(canvas, x, (int)y) = color;
        }
        // accumulate dy
        y += dy;
    }
}

double jc_signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
    return 0.5*((bx+ax)*(by-ay) + (cx+bx)*(cy-by) + (ax+cx)*(ay-cy));
}

void jc_draw_triangle(JC_Canvas canvas, int ax, int ay, int bx, int by, int cx, int cy, JC_Color color)
{
    int min_axbx = MIN(ax, bx);
    int bb_minx  = MIN(min_axbx, cx);
    int min_ayby = MIN(ay, by);
    int bb_miny  = MIN(min_ayby, cy);

    int max_axbx = MAX(ax, bx);
    int bb_maxx  = MAX(max_axbx, cx);
    int max_ayby = MAX(ay, by);
    int bb_maxy  = MAX(max_ayby, cy);
    double total_area = jc_signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 1) return;

    for (int x = bb_minx; x <= bb_maxx; x++) {
        for (int y = bb_miny; y <= bb_maxy; y++) {
            // The barycentric coord is proportional to the area of the triangle
            // made from P with the other two vertices. When the point is close
            // to a vertex the area made with the other vertices will be larger
            double alpha = jc_signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = jc_signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = jc_signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            // A negative coordinate means that the point is not in the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            if (x < 0 || x >= canvas.width || y < 0 || y >= canvas.height) continue;
            JC_PIXEL(canvas, x, y) = color;
        }
    }
}

void jc_set_shader_func(JC_ShaderFunc func, void *uniforms)
{
    jc_active_shader_func = func;
    jc_active_shader_uniforms = uniforms;
}

void jc_unset_shader_func(void)
{
    jc_active_shader_func = NULL;
    jc_active_shader_uniforms = NULL;
}

// void jc_rasterize_triangle(JC_Canvas canvas, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3)
// {
// }
//
// void jc_rasterize_triangle_ex(JC_Canvas canvas, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3, JC_Image texture,
//         JC_Color color)
// {
// }

void jc_draw_model(JC_Canvas canvas, JC_Model model, JC_Color color)
{
    int triangle_count = model.vertex_count / 3;
    for (int i = 0; i < triangle_count; i++) {
        JC_Vec3 v1 = model.vertices[3*i].position;
        JC_Vec3 v2 = model.vertices[3*i + 1].position;
        JC_Vec3 v3 = model.vertices[3*i + 2].position;

        // model transform
        v1 = jc_vec3_transform(model.transform, v1);
        v2 = jc_vec3_transform(model.transform, v2);
        v3 = jc_vec3_transform(model.transform, v3);

        // project to canvas
        JC_Vec2 p1 = jc_canvas_coord(canvas, v1);
        JC_Vec2 p2 = jc_canvas_coord(canvas, v2);
        JC_Vec2 p3 = jc_canvas_coord(canvas, v3);

        color = model.vertices[3*i].color;
        jc_draw_triangle(canvas, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
    }
}

// NOTE: This does not use any of the vertex colors or the model texture
void jc_draw_model_wires(JC_Canvas canvas, JC_Model model, JC_Color color)
{
    int triangle_count = model.vertex_count / 3;
    for (int i = 0; i < triangle_count; i++) {
        JC_Vec3 p1 = model.vertices[3*i].position;
        JC_Vec3 p2 = model.vertices[3*i + 1].position;
        JC_Vec3 p3 = model.vertices[3*i + 2].position;

        // model transform
        p1 = jc_vec3_transform(model.transform, p1);
        p2 = jc_vec3_transform(model.transform, p2);
        p3 = jc_vec3_transform(model.transform, p3);

        // project to canvas
        JC_Vec2 cp1 = jc_canvas_coord(canvas, p1);
        JC_Vec2 cp2 = jc_canvas_coord(canvas, p2);
        JC_Vec2 cp3 = jc_canvas_coord(canvas, p3);

        jc_draw_line(canvas, cp1.x, cp1.y, cp2.x, cp2.y, color);
        jc_draw_line(canvas, cp2.x, cp2.y, cp3.x, cp3.y, color);
        jc_draw_line(canvas, cp3.x, cp3.y, cp1.x, cp1.y, color);
    }
}

//------Color functions------
// Blend b onto a
JC_Color jc_color_blend_alpha(JC_Color ca, JC_Color cb)
{
    uint32_t alpha = cb.a;
    uint32_t r = (ca.r*(255 - alpha) + cb.r*alpha)/255;
    uint32_t g = (ca.g*(255 - alpha) + cb.g*alpha)/255;
    uint32_t b = (ca.b*(255 - alpha) + cb.b*alpha)/255;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    JC_Color c = { r, g, b, ca.a };
    return c;
}

JC_Color jc_color_mul(JC_Color ca, JC_Color cb)
{
    JC_Color c = {
        (uint32_t)ca.r * cb.r / 255,
        (uint32_t)ca.g * cb.g / 255,
        (uint32_t)ca.b * cb.b / 255,
        (uint32_t)ca.a * cb.a / 255,
    };
    return c;
}

JC_Color jc_color_from_int(uint32_t color)
{
    JC_Color c = {
        .r = (color >> 24) & 0xff,
        .g = (color >> 16) & 0xff,
        .b = (color >> 8)  & 0xff,
        .a = (color >> 0)  & 0xff,
    };
    return c;
}

//===========================
// Linear Algebra stuff
//==========================
// NOTE: I think this should be minimal and only provide the most important functions
// MATRIX: add, sub, mul, projection, translation, rotation, scale
// VECTOR: add, sub, transform, dot, cross, length, normalize

// Perspective projection of point p which should be the point after any model and view
// transformations
JC_Vec3 jc_project(JC_Vec3 p) {
    p.x /= p.z;
    p.y /= p.z;
    return p;
}

// Transform point from x: [-1, 1] to [0, width] and y: [-1, 1] to [height, 0]
JC_Vec2 jc_canvas_coord(JC_Canvas canvas, JC_Vec3 point)
{
    JC_Vec2 p;
    p.x = (point.x + 1.0f)/2.0f * canvas.width;
    p.y = (1.0f - (point.y + 1.0f)/2.0f) * canvas.height;
    return p;
}

// Vector functions
JC_Vec3 jc_vec3_add(JC_Vec3 a, JC_Vec3 b)
{
    JC_Vec3 v = { a.x + b.x, a.y + b.y, a.z + b.z };
    return v;
}

JC_Vec3 jc_vec3_sub(JC_Vec3 a, JC_Vec3 b)
{
    JC_Vec3 v = { a.x - b.x, a.y - b.y, a.z - b.z };
    return v;
}

JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v)
{
    JC_Vec3 u;
    float x = v.x;
    float y = v.y;
    float z = v.z;

    u.x = a.m11*x + a.m12*y + a.m13*z + a.m14;
    u.y = a.m21*x + a.m22*y + a.m23*z + a.m24;
    u.z = a.m31*x + a.m32*y + a.m33*z + a.m34;
    return u;
}

float jc_vec3_length(JC_Vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

JC_Vec3 jc_vec3_normalize(JC_Vec3 v)
{
    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) return v;
    v.x /= length;
    v.y /= length;
    v.z /= length;
    return v;
}

float jc_vec3_dot(JC_Vec3 a, JC_Vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

JC_Vec3 jc_vec3_cross(JC_Vec3 a, JC_Vec3 b)
{
    JC_Vec3 v = {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x 
    };
    return v;
}

// Matrix functions
JC_Matrix jc_matrix_identity(void)
{
    JC_Matrix m = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return m;
}

JC_Matrix jc_matrix_translate(float x, float y, float z)
{
    JC_Matrix m = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return m;
}

JC_Matrix jc_matrix_scale(float s)
{
    JC_Matrix m = {
        s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, 0.0f,
        0.0f, 0.0f, s, 0.0f,
        0.0f, 0.0f, 0.0f, s,
    };
    return m;
}

// TODO: rotate around any axis
JC_Matrix jc_matrix_rotate_x(float angle)
{
    JC_Matrix m = jc_matrix_identity();
    float c = cosf(angle);
    float s = sinf(angle);

    m.m22 = c;
    m.m32 = s;
    m.m23 = -s;
    m.m33 = c;

    return m;
}

JC_Matrix jc_matrix_rotate_y(float angle)
{
    JC_Matrix m = jc_matrix_identity();
    float c = cosf(angle);
    float s = sinf(angle);

    m.m11 = c;
    m.m13 = s;
    m.m31 = -s;
    m.m33 = c;

    return m;
}

JC_Matrix jc_matrix_rotate_z(float angle)
{
    JC_Matrix m = jc_matrix_identity();
    float c = cosf(angle);
    float s = sinf(angle);

    m.m11 = c;
    m.m21 = s;
    m.m12 = -s;
    m.m22 = c;

    return m;
}

JC_Matrix jc_matrix_add(JC_Matrix a, JC_Matrix b)
{
    JC_Matrix m;
    m.m11 = a.m11+b.m11; m.m12 = a.m12+b.m12; m.m13 = a.m13+b.m13; m.m14 = a.m14+b.m14;
    m.m21 = a.m21+b.m21; m.m22 = a.m22+b.m22; m.m23 = a.m23+b.m23; m.m24 = a.m24+b.m24;
    m.m31 = a.m31+b.m31; m.m32 = a.m32+b.m32; m.m33 = a.m33+b.m33; m.m34 = a.m34+b.m34;
    m.m41 = a.m41+b.m41; m.m42 = a.m42+b.m42; m.m43 = a.m43+b.m43; m.m44 = a.m44+b.m44;
    return m;
}

JC_Matrix jc_matrix_sub(JC_Matrix a, JC_Matrix b)
{
    JC_Matrix m;
    m.m11 = a.m11-b.m11; m.m12 = a.m12-b.m12; m.m13 = a.m13-b.m13; m.m14 = a.m14-b.m14;
    m.m21 = a.m21-b.m21; m.m22 = a.m22-b.m22; m.m23 = a.m23-b.m23; m.m24 = a.m24-b.m24;
    m.m31 = a.m31-b.m31; m.m32 = a.m32-b.m32; m.m33 = a.m33-b.m33; m.m34 = a.m34-b.m34;
    m.m41 = a.m41-b.m41; m.m42 = a.m42-b.m42; m.m43 = a.m43-b.m43; m.m44 = a.m44-b.m44;
    return m;
}

JC_Matrix jc_matrix_mul(JC_Matrix a, JC_Matrix b)
{
    JC_Matrix m = {0};
    m.m11 = a.m11*b.m11 + a.m12*b.m21 + a.m13*b.m31 + a.m14*b.m41;
    m.m12 = a.m11*b.m12 + a.m12*b.m22 + a.m13*b.m32 + a.m14*b.m42;
    m.m13 = a.m11*b.m13 + a.m12*b.m23 + a.m13*b.m33 + a.m14*b.m43;
    m.m14 = a.m11*b.m14 + a.m12*b.m24 + a.m13*b.m34 + a.m14*b.m44;

    m.m21 = a.m21*b.m11 + a.m22*b.m21 + a.m23*b.m31 + a.m24*b.m41;
    m.m22 = a.m21*b.m12 + a.m22*b.m22 + a.m23*b.m32 + a.m24*b.m42;
    m.m23 = a.m21*b.m13 + a.m22*b.m23 + a.m23*b.m33 + a.m24*b.m43;
    m.m24 = a.m21*b.m14 + a.m22*b.m24 + a.m23*b.m34 + a.m24*b.m44;

    m.m31 = a.m31*b.m11 + a.m32*b.m21 + a.m33*b.m31 + a.m34*b.m41;
    m.m32 = a.m31*b.m12 + a.m32*b.m22 + a.m33*b.m32 + a.m34*b.m42;
    m.m33 = a.m31*b.m13 + a.m32*b.m23 + a.m33*b.m33 + a.m34*b.m43;
    m.m34 = a.m31*b.m14 + a.m32*b.m24 + a.m33*b.m34 + a.m34*b.m44;

    m.m41 = a.m41*b.m11 + a.m42*b.m21 + a.m43*b.m31 + a.m44*b.m41;
    m.m42 = a.m41*b.m12 + a.m42*b.m22 + a.m43*b.m32 + a.m44*b.m42;
    m.m43 = a.m41*b.m13 + a.m42*b.m23 + a.m43*b.m33 + a.m44*b.m43;
    m.m44 = a.m41*b.m14 + a.m42*b.m24 + a.m43*b.m34 + a.m44*b.m44;
    return m;
}
#undef MIN
#undef MAX
#undef SWAP
#undef ABS
#undef da_append

#endif // JCANVAS_IMPLEMENTATION
//======================================
// Strip prefixes from types and functions. This idea is taken from https://github.com/tsoding/nob.h
// NOTE: the exception is canvas related functions such as jc_create, jc_destroy, jc_resize
#ifndef JC_PREFIX
#define Canvas JC_Canvas
#define Color JC_Color
#define Camera JC_Camera
#define Model JC_Model
#define Vertex JC_Vertex
#define Matrix JC_Matrix
#define Vec3 JC_Vec3
#define Vec2 JC_Vec2

#define load_ppm jc_load_ppm
#define save_ppm jc_save_ppm
#define model_load jc_model_load
#define model_load_from_memory jc_model_load_from_memory
#define model_destroy jc_model_destroy
#define blit jc_blit
#define blit_rect jc_blit_rect
#define fill jc_fill
#define draw_pixel jc_draw_pixel
#define draw_rect jc_draw_rect
#define draw_line jc_draw_line
#define draw_triangle jc_draw_triangle
#define draw_model jc_draw_model
#define draw_model_wires jc_draw_model_wires
#define color_from_int jc_color_from_int
#define color_blend_alpha jc_color_blend_alpha
#define clamp jc_clamp
#define project jc_project
#define canvas_coord jc_canvas_coord
#define vec3_add jc_vec3_add
#define vec3_sub jc_vec3_sub
#define vec3_transform jc_vec3_transform
#define vec3_normalize jc_vec3_normalize
#define vec3_cross jc_vec3_cross
#define vec3_dot jc_vec3_dot
#define vec3_length jc_vec3_length
#define matrix_identity jc_matrix_identity
#define matrix_translate jc_matrix_translate
#define matrix_scale jc_matrix_scale
#define matrix_rotate_x jc_matrix_rotate_x
#define matrix_rotate_y jc_matrix_rotate_y
#define matrix_rotate_z jc_matrix_rotate_z
#define matrix_add jc_matrix_add
#define matrix_sub jc_matrix_sub
#define matrix_mul jc_matrix_mul
#endif

#endif // JCANVAS_H

