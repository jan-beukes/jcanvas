// jcanvas is a simple software rendering library for drawing into a pixel buffer
// Most of the rendering techniques were learned from https://haqr.eu/tinyrenderer
// Raylib source code was also used as a reference for things such as the Matrix functions

// This is an STB style single header library.
// One of your source files must define JCANVAS_IMPLEMENTATION before this header is included.
// This will include all of the implementation into that source file
// Otherwise the header can be included as normal everywhere else in your program

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
} JC_Color;

// NOTE: This is taken from raylib https://github.com/raysan5/raylib
// MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized with { }
// This is called aggregate initialization (C++11 feature)
#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
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

    JC_Color diffuse;
    JC_Canvas texture;
} JC_Model;

#define JC_PIXEL(c, x, y) (c).pixels[y*(c).width + x]

// Mr C++ don't mangle my function names
#if defined(__cplusplus)
extern "C" {
#endif

bool jc_create(JC_Canvas *canvas, int width, int height);
void jc_destroy(JC_Canvas *canvas);
void jc_resize(JC_Canvas *canvas, int w, int h);

bool jc_load_ppm(JC_Image *canvas, const char *path);
bool jc_save_ppm(JC_Image canvas, const char *path);

// load a model from obj data
bool jc_load_obj(JC_Model *model, const char *path);
bool jc_load_obj_from_memory(JC_Model *model, char *data);

// clamps x and y to the canvas
void jc_clamp(JC_Canvas canvas, int *x, int *y);
// These functions are used to draw another canvas/image to a canvas
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y);
// Like cv_blit but it scales the image so that it occupies the rectangle (x, y, w, h)
void jc_blit_rect(JC_Canvas canvas, JC_Image image, int x, int y, int w, int h);

void jc_fill(JC_Canvas canvas, JC_Color color);
void jc_pixel(JC_Canvas canvas, int x, int y, JC_Color color);
void jc_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color);
void jc_line(JC_Canvas canvas, int x1, int y1, int x2, int y2, JC_Color color);

void jc_model(JC_Canvas canvas, JC_Model model, JC_Color color);
void jc_model_wires(JC_Canvas canvas, JC_Model model, JC_Color color);

JC_Vec2 jc_canvas_coord(JC_Canvas, JC_Vec3 point);
JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v);
JC_Matrix jc_matrix_identity(void);
JC_Matrix jc_matrix_mul(JC_Matrix a, JC_Matrix b);

// converts a color stored in an integer to a color struct
// 0xaa00aaff will be a purple color as the LSB end is assumed stores alpha
JC_Color jc_color_from_int(uint32_t color);
JC_Color jc_color_blend_alpha(JC_Color a, JC_Color b);

#if defined(__cplusplus)
}
#endif

#ifdef JCANVAS_IMPLEMENTATION
//======================================
// IMPLEMENTATION
//======================================

// TODO: maybe make these part of the API?
#define _MIN(x, y) ((x) < (y) ? (x) : (y))
#define _MAX(x, y) ((x) > (y) ? (x) : (y))
#define _SWAP(x, y) do { x ^= y; y ^= x; x ^= y; } while(0)
#define _ABS(x) ((x) > 0 ? (x) : -(x))

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
    char *buf = malloc(file_size);
    if (buf == NULL) return NULL;
    fread(buf, file_size, 1, file);
    if (size) *size = file_size;

    fclose(file);
    return buf;
}

// load a ppm file
bool jc_load_ppm(JC_Canvas *canvas, const char *path)
{
    long size;
    char *data = _read_entire_file(path, &size);
    if (data == NULL) return false;

    char *b = data;
    if (*b++ != 'P' || *b++ != '6') return false;
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
    canvas->pixels = malloc(width*height*sizeof(JC_Color));

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
        canvas->pixels[i] = c;
    }
    free(data);
    return true;
}

bool jc_save_ppm(JC_Canvas canvas, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return false;
    }
    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n%d\n", canvas.width, canvas.height, 255);
    for (int i = 0; i < canvas.width*canvas.height; i++) {
        JC_Color color = canvas.pixels[i];
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

bool jc_load_obj_from_memory(JC_Model *model, char *data)
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
        while (!isspace(*s)) s++;
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
                if (*end != '/') {
                    da_append(&vertices, vertex);
                    continue;
                }
            }
        } else {
            // TODO: handle other things https://en.wikipedia.org/wiki/Wavefront_.obj_file
        }
    }
    free(positions.items);
    free(texcoords.items);
    free(normals.items);

    model->vertex_count = vertices.count;
    model->vertices = realloc(vertices.items, model->vertex_count * sizeof(JC_Vertex));
    model->transform = jc_matrix_identity();
    model->diffuse = WHITE;
    return true;
}

bool jc_load_obj(JC_Model *model, const char *path)
{
    long size;
    char *data = _read_entire_file(path, &size);
    if (data == NULL) return false;
    bool result = jc_load_obj_from_memory(model, data);
    free(data);
    return result;
}

//===============
// Rendering
//===============

void jc_clamp(JC_Canvas canvas, int *x, int *y)
{
    int x_clamped = _MAX(*x, 0);
    *x = _MIN(x_clamped, canvas.width);
    int y_clamped = _MAX(*y, 0);
    *y = _MIN(y_clamped, canvas.height);
}

// TODO: look into different ways of alpha blending
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y)
{
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

void jc_pixel(JC_Canvas canvas, int x, int y, JC_Color color)
{
    JC_PIXEL(canvas, x, y) = color;
}

void jc_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color)
{
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

void jc_line(JC_Canvas canvas, int x1, int y1, int x2, int y2, JC_Color color)
{
    // if steep we swap x and y to iterate over y
    bool steep = _ABS(x1 - x2) < _ABS(y1 - y2);
    if (steep) {
        _SWAP(x1, y1);
        _SWAP(x2, y2);
    }
    // make sure that x1 is the smallest of the points
    if (x1 > x2) {
        _SWAP(x1, x2);
        _SWAP(y1, y2);
    }

    float y = y1;
    float dy = (float)(y2-y1) / (x2-x1);
    for (int x = x1; x <= x2; ++x) {
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

//==================
// 3D Rendering
//=================

void jc_model(JC_Canvas canvas, JC_Model model, JC_Color color)
{
    // TODO: implement
    jc_model_wires(canvas, model, color);
}

void jc_model_wires(JC_Canvas canvas, JC_Model model, JC_Color color)
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

        jc_line(canvas, cp1.x, cp1.y, cp2.x, cp2.y, color);
        jc_line(canvas, cp2.x, cp2.y, cp3.x, cp3.y, color);
        jc_line(canvas, cp3.x, cp3.y, cp1.x, cp1.y, color);
    }
}

//======================
// Linear Algebra stuff
//=====================

// Transform point from x: [-1, 1] to [0, width] and y: [-1, 1] to [height, 0]
JC_Vec2 jc_canvas_coord(JC_Canvas canvas, JC_Vec3 point)
{
    JC_Vec2 p;
    p.x = (point.x + 1.0f)/2.0f * canvas.width;
    p.y = (1.0f - (point.y + 1.0f)/2.0f) * canvas.height;
    return p;
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
    JC_Matrix m = jc_matrix_identity();
    m.m14 = x;
    m.m14 = y;
    m.m14 = z;
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

#endif // JCANVAS_IMPLEMENTATION
//======================================
// TODO: strip prefixes from types and math/util functions similar to https://github.com/tsoding/nob.h
// #ifndef JC_PREFIX
// #define Color JC_Color

#endif // JCANVAS_H

