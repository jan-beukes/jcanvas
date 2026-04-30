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
//
// OpenMP can be enabled for some functions with -fopenmp.

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


enum JC_Wrap {
    JC_WRAP_REPEAT,
    JC_WRAP_CLAMP,
};

enum JC_Filter {
    JC_FILTER_NEREAST,
    JC_FILTER_BILINEAR,
};

// The main canvas/image structure in RGBA8 format
typedef struct {
    int width, height;
    JC_Color *pixels;
    int stride; // number of bytes per row

    // This is for filtering and wraping
    // see JC_ImageFlags
    int flags;
} JC_Canvas;

// This typedef is just a way to document usage since these are the exact same structure
// There are also aliases of all the jc_canvas* functions such as jc_image_create
typedef JC_Canvas JC_Image;

typedef struct {
    float x, y;
} JC_Vec2;

typedef struct {
    float x, y, z;
} JC_Vec3;

typedef struct {
    float x, y, z, w;
} JC_Vec4;

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

typedef struct {
    float x, y;
    float w, h;
} JC_Rect;

// You can change these
#ifndef JC_NEAR_PLANE
#define JC_NEAR_PLANE 0.1f
#endif
#ifndef JC_FAR_PLANE
#define JC_FAR_PLANE 100.0f
#endif

typedef enum {
    JC_PERSPECTIVE,
    JC_ORTHOGRAPHIC,
} JC_Projection;

typedef struct {
    JC_Vec3 position;
    JC_Vec3 target;
    JC_Vec3 up;
    float fov; // NOTE: This is in radians
    JC_Projection projection;
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

// This is a "shader" that is called for every pixel during rasterization
// input will hold the interpolated values of the vertices given to the rasterize function
// It should return TRUE if the pixel was set or FALSE to discard
typedef bool (*JC_ShaderFunc)(JC_Color *out, JC_Vertex input, void *uniforms);

#define JC_PIXEL(c, x, y) (c).pixels[(y)*(c).stride + (x)]
#define JC_IN_BOUNDS(c, x, y) ((x) >= 0 && (x) < (c).width && (y) >= 0 && (y) < (c).height)
#define JC_IMAGE_WRAP(flags) ((flags << 8) & 0xff)
#define JC_IMAGE_FILTER(flags) (flags & 0xff)

// Mr C++ don't mangle my function names
#if defined(__cplusplus)
extern "C" {
#endif

bool jc_canvas_create(JC_Canvas *canvas, int width, int height);
JC_Canvas jc_subcanvas(JC_Canvas canvas, int x, int y, int w, int h);
void jc_canvas_destroy(JC_Canvas *canvas);
void jc_canvas_resize(JC_Canvas *canvas, int w, int h);

// These are currently the only image loading functions provided
// If you want to load an image then you can simply load the data using something like stb_image
// and manually initialize the canvas pixels, width and height or use jc_create and fill the pixels
bool jc_load_ppm(JC_Image *image, const char *path);
bool jc_save_ppm(JC_Image image, const char *path);

// TODO: add QOI https://github.com/phoboslab/qoi/blob/master/qoi.h
// load_qoi load_qoi_from_memory and save_qoi

// This is for the obj format!
// Similar to image loading the Model object is not very complicated and you can easily use some
// other library to load a model and then convert the vertices to JC_Vertex (or just convert to obj)
// NOTE: only triangulated meshes are supported.
bool jc_model_load(JC_Model *model, const char *path);
bool jc_model_load_from_memory(JC_Model *model, char *data, long size);
void jc_model_destroy(JC_Model *model);

// These functions are used to draw another canvas/image to a canvas
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y);
// Like cv_blit but it scales the image so that it occupies the rectangle (x, y, w, h)
void jc_blit_rect(JC_Canvas canvas, JC_Image image, JC_Rect dst, JC_Rect src);

// TODO: vector argument functions?
void jc_fill(JC_Canvas canvas, JC_Color color);
void jc_draw_pixel(JC_Canvas canvas, int x, int y, JC_Color color);
void jc_draw_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color);
void jc_draw_line(JC_Canvas canvas, int ax, int ay, int bx, int by, JC_Color color);
void jc_draw_triangle(JC_Canvas canvas, int ax, int ay, int bx, int by, int cx, int cy, JC_Color color);

// 3D rendering
// This stores the canvas, camera and zbuffer internally and also clears the zbuffer
void jc_begin_mode_3d(JC_Canvas canvas, JC_Camera camera, float *zbuffer);
void jc_end_mode_3d(void);

// If you want to disable any writes to the zbuffer
void jc_disable_depth(void);
void jc_enable_depth(void);

// The shader func is only called in the rasterize functions
void jc_set_shader(JC_ShaderFunc func, void *uniforms);
void jc_set_shader_uniforms(void *uniforms);
void jc_unset_shader(void);
JC_Color jc_image_sample(JC_Image image, float u, float v);

// This is the main rasterization function called to draw triangles. Input is vertices after
// projection onto the screen. Y will be up so this function flips y.
void jc_rasterize_triangle(JC_Canvas canvas, float *zbuffer, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3,
        JC_Image texture, JC_Color color);

// These are the function used to dispatch a render. They are used by all the 3d drawing functions
// The zbuffer is what is used for depth testing so make sure to use the same one for the entire scene
// It should be of size width*height and initialized and cleared to zero. After projection z is in [0, 255]
void jc_render_geometry(JC_Canvas canvas, float *zbuffer, JC_Matrix mvp, JC_Vertex *vertices, int vertex_count);
void jc_render_geometry_ex(JC_Canvas canvas, float *zbuffer, JC_Matrix mvp,
        JC_Vertex *vertices, int vertex_count, JC_Image texture, JC_Color color);
void jc_render_geometry_lines(JC_Canvas canvas, JC_Matrix mvp, JC_Vertex *vertices, int vertex_count,
        JC_Color color);

void jc_draw_model(JC_Model model, JC_Vec3 position, JC_Color color);
void jc_draw_model_wires(JC_Model model, JC_Vec3 position, JC_Color color);

JC_Vertex jc_barycentric_interpolate(JC_Vertex v1, JC_Vertex v2, JC_Vertex v3, float alpha, float beta, float gamma);
// the integer is assumed RGBA with R being the big end
JC_Color jc_color_from_int(uint32_t color);
JC_Color jc_color_blend_alpha(JC_Color a, JC_Color b);
JC_Color jc_color_add(JC_Color a, JC_Color b);
JC_Color jc_color_mul(JC_Color a, JC_Color b);
JC_Color jc_color_scale(JC_Color a, float s);
JC_Color jc_color_lerp(JC_Color a, JC_Color b, float t);

//==========Math Functions========
#define JC_DEG2RAD(x) (M_PI * x / 180)
#define JC_RAD2DEG(x) (180 * x / M_PI)
#define JC_MIN(x, y) ((x) < (y) ? (x) : (y))
#define JC_MAX(x, y) ((x) > (y) ? (x) : (y))
#define JC_ABS(x) ((x) > 0 ? (x) : -(x))
#define JC_SWAP(x, y) do { \
    __typeof__(x) tmp = x; \
    x = y; \
    y = tmp; \
} while (0)

#define JC_CLAMP(c, x, y) do { \
    int x_clamped = JC_MAX(x, 0); \
    x = JC_MIN(x_clamped, (c).width - 1); \
    int y_clamped = JC_MAX(y, 0); \
    y = JC_MIN(y_clamped, (c).height - 1); \
} while (0)

JC_Vec4 jc_vec4_transform(JC_Matrix a, JC_Vec3 v);
JC_Vec4 jc_vec4_lerp(JC_Vec4 a, JC_Vec4 b, float t);

JC_Vec3 jc_vec3_add(JC_Vec3 a, JC_Vec3 b);
JC_Vec3 jc_vec3_sub(JC_Vec3 a, JC_Vec3 b);
JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v);
JC_Vec3 jc_vec3_normalize(JC_Vec3 v);
JC_Vec3 jc_vec3_cross(JC_Vec3 a, JC_Vec3 b);
JC_Vec3 jc_vec3_lerp(JC_Vec3 a, JC_Vec3 b, float t);
float jc_vec3_dot(JC_Vec3 a, JC_Vec3 b);
float jc_vec3_length(JC_Vec3 v);
float jc_vec3_angle(JC_Vec3 a, JC_Vec3 b);

// TODO: Vector2 functions

JC_Matrix jc_matrix_identity(void);
JC_Matrix jc_matrix_orthographic(float left, float right, float bottom, float top, float near, float far);
JC_Matrix jc_matrix_perspective(float fov, float aspect, float near, float far);
JC_Matrix jc_matrix_viewport(float width, float height);
JC_Matrix jc_matrix_view(JC_Camera camera);

JC_Matrix jc_matrix_translate(float x, float y, float z);
JC_Matrix jc_matrix_scale(float x, float y, float z);
JC_Matrix jc_matrix_rotate(JC_Vec3 axis, float angle);
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

// internal state
static struct {
    bool mode_3d_active;
    bool depth_disabled;
    JC_Canvas canvas;
    JC_Matrix view_proj; // this is instead of camera
    float *zbuffer;
    JC_ShaderFunc active_shader;
    void *active_shader_uniforms;
} _jc_state;

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

bool jc_canvas_create(JC_Canvas *canvas, int width, int height)
{
    canvas->width = width;
    canvas->height = height;
    canvas->stride = width;
    size_t size = width * height * sizeof(JC_Color);
    canvas->pixels = malloc(size);
    if (canvas->pixels == NULL) return false;
    for (int i = 0; i < width*height; ++i) {
        JC_Color c = { 0, 0, 0, 255 };
        canvas->pixels[i] = c;
    }
    return true;
}

JC_Canvas jc_subcanvas(JC_Canvas canvas, int x, int y, int w, int h)
{
    int x_max = x + w;
    int y_max = y + h;
    JC_CLAMP(canvas, x, y);
    JC_CLAMP(canvas, x_max, y_max);

    JC_Canvas c = canvas;
    c.pixels = &JC_PIXEL(canvas, x, y);
    c.width = x_max - x;
    c.height = y_max - y;
    return c;
}

void jc_canvas_destroy(JC_Canvas *canvas)
{
    // This is a subcanvas and we cannot be certain that the pointer is a valid free
    if (canvas->width != canvas->stride) return;
    free(canvas->pixels);
    canvas->pixels = NULL;
}

void jc_canvas_resize(JC_Canvas *canvas, int w, int h)
{
    canvas->pixels = realloc(canvas->pixels, w*h*sizeof(JC_Color));
    canvas->width = w;
    canvas->stride = w;
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

    image->width = width;
    image->height = height;
    image->stride = width;
    image->pixels = malloc(width*height*sizeof(JC_Color));

    // parse pixels
    uint8_t *image_pixels = (uint8_t*)b;
    for (int i = 0; i < width*height; ++i) {
        JC_Color c = {0};
        // 2 bytes
        if (maxval > 255) {
            int r = (image_pixels[6*i] << 8) | image_pixels[6*i + 1];
            int g = (image_pixels[6*i + 2] << 8) | image_pixels[6*i + 3];
            int b = (image_pixels[6*i + 4] << 8) | image_pixels[6*i + 5];
            c.r = 255 * r / maxval;
            c.g = 255 * g / maxval;
            c.b = 255 * b / maxval;
        } else {
            c.r = image_pixels[3*i];
            c.g = image_pixels[3*i + 1];
            c.b = image_pixels[3*i + 2];
        }
        c.a = 255;
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
    while((line = strtok_r(ptr, "\r\n", &ptr))) {
        char type[16];
        sscanf(line, "%s", type);
        if (strcmp(type, "#") == 0) continue;

        char *s = line;
        while(s && isspace(*s)) s++;
        while ((s - data < size) && !isspace(*s)) s++;

        if (strcmp(type, "v") == 0) {
            float x, y, z, w;
            int n = sscanf(s, "%f %f %f %f", &x, &y, &z, &w);
            if (n == 4) {
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
                v = strtol(s, &end, 0);
                vertex.position = positions.items[v-1];
                s = end + 1;
                if (*end != '/') {
                    da_append(&vertices, vertex);
                    continue;
                }
                // texcoord
                vt = strtol(s, &end, 0);
                vertex.texcoord = texcoords.items[vt-1];
                s = end + 1;
                if (*end != '/') {
                    da_append(&vertices, vertex);
                    continue;
                }
                // normal
                vn = strtol(s, &end, 0);
                vertex.normal = normals.items[vn-1];
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
    jc_canvas_destroy(&model->texture);
}

//===============================================
// RENDERING
//===============================================

// TODO: look into different ways of blending (https://wiki.libsdl.org/SDL3/SDL_BlendMode)
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y)
{
    if ((x < 0 && x+image.width < 0) || (x >= canvas.width && x+image.width >= canvas.width)) return;
    if ((y < 0 && y+image.height < 0) || (y >= canvas.height && y+image.height >= canvas.height)) return;

    int x_min = x, y_min = y;
    int x_max = x + image.width, y_max = y + image.height;
    JC_CLAMP(canvas, x_min, y_min);
    JC_CLAMP(canvas, x_max, y_max);
    for (int i = y_min; i <= y_max; ++i) {
        for (int j = x_min; j <= x_max; ++j) {
            if (!JC_IN_BOUNDS(image, j-x, i-y)) continue;

            JC_Color image_pixel = JC_PIXEL(image, j - x, i - y);
            JC_Color canvas_pixel = JC_PIXEL(canvas, j, i);
            JC_PIXEL(canvas, j, i) = jc_color_blend_alpha(canvas_pixel, image_pixel);
        }
    }
}

// TODO: Use the interpolation
void jc_blit_rect(JC_Canvas canvas, JC_Image image, JC_Rect dst, JC_Rect src)
{
    if ((dst.x < 0 && dst.x+dst.w < 0) || (dst.x >= canvas.width && dst.x+dst.w >= canvas.width)) return;
    if ((dst.y < 0 && dst.y+dst.h < 0) || (dst.y >= canvas.height && dst.y+dst.h >= canvas.height)) return;

    if ((src.x < 0 && src.x+src.w < 0) || (src.x >= image.width && src.x+src.w >= image.width)) return;
    if ((src.y < 0 && src.y+src.h < 0) || (src.y >= image.height && src.y+src.h >= image.height)) return;

    int x_min = dst.x, y_min = dst.y;
    int x_max = dst.x + dst.w, y_max = dst.y + dst.h;
    JC_CLAMP(canvas, x_min, y_min);
    JC_CLAMP(canvas, x_max, y_max);
    for (int i = y_min; i <= y_max; ++i) {
        for (int j = x_min; j <= x_max; ++j) {
            int image_x = src.x + (j - dst.x) * src.w / dst.w;
            int image_y = src.y + (i - dst.y) * src.h / dst.h;
            if (!JC_IN_BOUNDS(image, image_x, image_y)) continue;

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
    if ((x < 0 && x+w < 0) || (x >= canvas.width && x+w >= canvas.width)) return;
    if ((y < 0 && y+h < 0) || (y >= canvas.height && y+h >= canvas.height)) return;

    int x_max = x + w, y_max = y + h;
    JC_CLAMP(canvas, x, y);
    JC_CLAMP(canvas, x_max, y_max);
    for (int i = y; i <= y_max; ++i) {
        for (int j = x; j <= x_max; ++j) {
            JC_PIXEL(canvas, j, i) = color;
        }
    }
}

void jc_draw_line(JC_Canvas canvas, int ax, int ay, int bx, int by, JC_Color color)
{
    // if steep we swap x and y to iterate over y
    bool steep = JC_ABS(ax - bx) < JC_ABS(ay - by);
    if (steep) {
        JC_SWAP(ax, ay);
        JC_SWAP(bx, by);
    }
    // make sure that ax is the smallest of the points
    if (ax > bx) {
        JC_SWAP(ax, bx);
        JC_SWAP(ay, by);
    }

    float y = ay;
    float dy = (float)(by-ay) / (bx-ax);
    for (int x = ax; x <= bx; ++x) {
        // since y is actually x in this case
        int yi = (int)y;
        if (steep) {
            if (JC_IN_BOUNDS(canvas, yi, x))
                JC_PIXEL(canvas, yi, x) = color;
        } else {
            if (JC_IN_BOUNDS(canvas, x, yi))
                JC_PIXEL(canvas, x, yi) = color;
        }
        // accumulate dy
        y += dy;
    }
}

// https://en.wikipedia.org/wiki/Shoelace_formula#Triangle_formula
double jc_signed_triangle_area(float ax, float ay, float bx, float by, float cx, float cy)
{
    return 0.5*((ax*by - ay*bx) + (bx*cy - by*cx) + (cx*ay - cy*ax));
}

void jc_draw_triangle(JC_Canvas canvas, int ax, int ay, int bx, int by, int cx, int cy, JC_Color color)
{
    int min_axbx = JC_MIN(ax, bx);
    int bb_minx  = JC_MIN(min_axbx, cx);
    int min_ayby = JC_MIN(ay, by);
    int bb_miny  = JC_MIN(min_ayby, cy);
    JC_CLAMP(canvas, bb_minx, bb_miny);

    int max_axbx = JC_MAX(ax, bx);
    int bb_maxx  = JC_MAX(max_axbx, cx);
    int max_ayby = JC_MAX(ay, by);
    int bb_maxy  = JC_MAX(max_ayby, cy);
    JC_CLAMP(canvas, bb_maxx, bb_maxy);

    double total_area = jc_signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 1) return;

    for (int x = bb_minx; x <= bb_maxx; x++) {
        for (int y = bb_miny; y <= bb_maxy; y++) {
            // The barycentric coord is proportional to the area of the triangle
            // made from P with the other two vertices. When the point is close
            // to a vertex the area made with the other vertices will be larger
            double alpha = jc_signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = jc_signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = 1.0 - alpha - beta;
            // A negative coordinate means that the point is not in the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            if (!JC_IN_BOUNDS(canvas, x, y)) continue;
            JC_PIXEL(canvas, x, y) = color;
        }
    }
}

//-----3D state management-----
void jc_begin_mode_3d(JC_Canvas canvas, JC_Camera camera, float *zbuffer)
{
    if (zbuffer == NULL || canvas.pixels == NULL) return;

    if (!_jc_state.depth_disabled) {
        memset(zbuffer, 0, canvas.stride*canvas.height*sizeof(*zbuffer));
    }
    _jc_state.canvas = canvas;

    JC_Matrix view = jc_matrix_view(camera);
    JC_Matrix projection;
    float aspect = (float)canvas.width / canvas.height;
    float fov = camera.fov == 0.0f ? JC_DEG2RAD(60) : camera.fov;
    if (camera.projection == JC_PERSPECTIVE) {
        projection = jc_matrix_perspective(fov, aspect, JC_NEAR_PLANE, JC_FAR_PLANE);
    } else {
        float top = camera.fov/2.0f;
        float right = top*aspect;
        projection = jc_matrix_orthographic(-right, right, -top, top, JC_NEAR_PLANE, JC_FAR_PLANE);
    }

    _jc_state.view_proj = jc_matrix_mul(projection, view);
    _jc_state.zbuffer = zbuffer;
    _jc_state.mode_3d_active = true;
}

void jc_end_mode_3d(void)
{
    _jc_state.mode_3d_active = false;
    _jc_state.view_proj = jc_matrix_identity();
    _jc_state.zbuffer = NULL;
}

void jc_disable_depth(void)
{
    _jc_state.depth_disabled = true;
}

void jc_enable_depth(void)
{
    _jc_state.depth_disabled = false;
}

void jc_set_shader(JC_ShaderFunc func, void *uniforms)
{
    _jc_state.active_shader = func;
    _jc_state.active_shader_uniforms = uniforms;
}

void jc_unset_shader(void)
{
    _jc_state.active_shader = NULL;
    _jc_state.active_shader_uniforms = NULL;
}

void jc_set_shader_uniforms(void *uniforms)
{
    _jc_state.active_shader_uniforms = uniforms;
}

JC_Color jc_image_sample(JC_Image image, float u, float v)
{
    int wrap = JC_IMAGE_WRAP(image.flags);
    // TODO: filtering
    // int filtering = JC_IMAGE_FILTER(image.flags);
    int x = u * (image.width - 1);
    int y = (1.0f - v) * (image.height - 1);
    switch (wrap) {
        case JC_WRAP_REPEAT:
            x = ((x % image.width) + image.width) % image.width;
            y = ((y % image.height) + image.height) % image.height;
            break;
        case JC_WRAP_CLAMP:
            JC_CLAMP(image, x, y);
            break;
        default:
            return MAGENTA;
    }
    JC_Color out = JC_PIXEL(image, x, y);
    return out;
}

// rasterize a single triangle
void jc_rasterize_triangle(JC_Canvas canvas, float *zbuffer, JC_Vertex v1, JC_Vertex v2, JC_Vertex v3,
        JC_Image texture, JC_Color color)
{
    // TODO: OPTIMIZE
    float ax = v1.position.x, ay = v1.position.y;
    float bx = v2.position.x, by = v2.position.y;
    float cx = v3.position.x, cy = v3.position.y;

    float min_axbx = JC_MIN(ax, bx);
    float bb_minx = JC_MIN(min_axbx, cx);
    float min_ayby = JC_MIN(ay, by);
    float bb_miny  = JC_MIN(min_ayby, cy);
    JC_CLAMP(canvas, bb_minx, bb_miny);

    float max_axbx = JC_MAX(ax, bx);
    float bb_maxx  = JC_MAX(max_axbx, cx);
    float max_ayby = JC_MAX(ay, by);
    float bb_maxy  = JC_MAX(max_ayby, cy);
    JC_CLAMP(canvas, bb_maxx, bb_maxy);

    double total_area = jc_signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 0.05) return;
    bool has_texture = texture.pixels != NULL;

    for (int x = bb_minx; x <= bb_maxx; x++) {
        for (int y = bb_miny; y <= bb_maxy; y++) {
            // The barycentric coord is proportional to the area of the triangle
            // made from P with the other two vertices. When the point is close
            // to a vertex the area made with the other vertices will be larger
            double alpha = jc_signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = jc_signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = 1.0 - alpha - beta;
            // A negative coordinate means that the point is not in the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            JC_Vertex frag = jc_barycentric_interpolate(v1, v2, v3, alpha, beta, gamma);
            float zbuf = zbuffer[y*canvas.width + x];

            if (frag.position.z >= 1.0f) continue;
            if (frag.position.z <= zbuf) continue;

            if (!_jc_state.depth_disabled) {
                zbuffer[y*canvas.width + x] = frag.position.z;
            }

            // get back u, v from perspective corrected space by dividing by 1 / z
            frag.texcoord.x /= frag.texcoord.z;
            frag.texcoord.y /= frag.texcoord.z;

            frag.color = jc_color_mul(frag.color, color);
            JC_Color out = frag.color;
            if (_jc_state.active_shader != NULL) {
                bool discard = !_jc_state.active_shader(&out, frag, _jc_state.active_shader_uniforms);
                if (discard) {
                    zbuffer[y*canvas.width + x] = zbuf;
                    continue;
                }
            } else if (has_texture) {
                out = jc_image_sample(texture, frag.texcoord.x, frag.texcoord.y);
            }

            // NOTE: here we flip to the screen
            JC_PIXEL(canvas, x, (canvas.height-1 - y)) = out;
        }
    }
}

void jc_render_geometry(JC_Canvas canvas, float *zbuffer, JC_Matrix mvp, JC_Vertex *vertices, int vertex_count)
{
    JC_Image image = {0};
    jc_render_geometry_ex(canvas, zbuffer, mvp, vertices, vertex_count, image, WHITE);
}


void jc_render_geometry_ex(JC_Canvas canvas, float *zbuffer, JC_Matrix mvp,
        JC_Vertex *vertices, int vertex_count, JC_Image texture, JC_Color color)
{
    JC_Matrix viewport = jc_matrix_viewport(canvas.width, canvas.height);
    int triangle_count = vertex_count / 3;
#pragma omp parallel for
    for (int i = 0; i < triangle_count; i++) {
        JC_Vertex v[3];
        v[0] = vertices[3*i];
        v[1] = vertices[3*i + 1];
        v[2] = vertices[3*i + 2];

        // transform to clip space
        JC_Vec4 clip[3];
        clip[0] = jc_vec4_transform(mvp, v[0].position);
        clip[1] = jc_vec4_transform(mvp, v[1].position);
        clip[2] = jc_vec4_transform(mvp, v[2].position);

        // handle near plane clipping
        // TODO: Properly handle splitting the triangles: https://youtu.be/MMB6pfJsx64
        int clip_count = 0;
        for (int i = 0; i < 3; i++) {
            if (clip[i].w <= JC_NEAR_PLANE) {
                clip_count++;
                clip[i].w = JC_NEAR_PLANE;
            }
        }
        if (clip_count >= 1) continue;
        for (int i = 0; i < 3; i++) {
            // transform before sending off to rasterization
            // perspective division
            v[i].position.x = clip[i].x / clip[i].w;
            v[i].position.y = clip[i].y / clip[i].w;
            v[i].position.z = clip[i].z / clip[i].w;
            // make sure that when we interpolate it is in perspective corrected space
            // texcoor.z will store 1/z which we can then multiply with to get the 
            v[i].texcoord.x /= clip[i].w;
            v[i].texcoord.y /= clip[i].w;
            v[i].texcoord.z = 1.0f / clip[i].w;

            v[i].normal = jc_vec3_transform(mvp, v[i].normal);
            v[i].position = jc_vec3_transform(viewport, v[i].position);
        }
        
        jc_rasterize_triangle(canvas, zbuffer, v[0], v[1], v[2], texture, color);
    }
}

void jc_render_geometry_lines(JC_Canvas canvas, JC_Matrix mvp, JC_Vertex *vertices, int vertex_count,
        JC_Color color)
{
    JC_Matrix viewport = jc_matrix_viewport(canvas.width, canvas.height);
    int triangle_count = vertex_count / 3;
#pragma omp parallel for
    for (int i = 0; i < triangle_count; i++) {
        JC_Vec3 p1 = vertices[3*i].position;
        JC_Vec3 p2 = vertices[3*i + 1].position;
        JC_Vec3 p3 = vertices[3*i + 2].position;

        // transform to clip space
        JC_Vec4 clip[3];
        clip[0] = jc_vec4_transform(mvp, p1);
        clip[1] = jc_vec4_transform(mvp, p2);
        clip[2] = jc_vec4_transform(mvp, p3);

        JC_Vec3 ndc[3];
        int near_clip_count = 0;
        for (int i = 0; i < 3; i++) {
            float w = clip[i].w;
            if (w <= JC_NEAR_PLANE) {
                w = JC_NEAR_PLANE;
                near_clip_count++;
            }
            ndc[i].x = clip[i].x / w;
            ndc[i].y = clip[i].y / w;
            ndc[i].z = clip[i].z / w;
        }
        if (near_clip_count == 3) continue;
        p1 = jc_vec3_transform(viewport, ndc[0]);
        p2 = jc_vec3_transform(viewport, ndc[1]);
        p3 = jc_vec3_transform(viewport, ndc[2]);

        jc_draw_line(canvas, p1.x, canvas.height - p1.y, p2.x, canvas.height - p2.y, color);
        jc_draw_line(canvas, p2.x, canvas.height - p2.y, p3.x, canvas.height - p3.y, color);
        jc_draw_line(canvas, p3.x, canvas.height - p3.y, p1.x, canvas.height - p1.y, color);
    }
}

void jc_draw_model(JC_Model model, JC_Vec3 position, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;
    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;
    JC_Matrix view_proj = _jc_state.view_proj;

    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix mat_model = jc_matrix_mul(model.transform, translate);
    JC_Matrix mvp = jc_matrix_mul(view_proj, mat_model);
    jc_render_geometry_ex(canvas, zbuffer, mvp, model.vertices, model.vertex_count, model.texture, color);
}

// NOTE: This does not use any of the vertex colors or the model texture
void jc_draw_model_wires(JC_Model model, JC_Vec3 position, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;
    JC_Canvas canvas = _jc_state.canvas;
    JC_Matrix view_proj = _jc_state.view_proj;

    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix mat_model = jc_matrix_mul(model.transform, translate);
    JC_Matrix mvp = jc_matrix_mul(view_proj, mat_model);
    jc_render_geometry_lines(canvas, mvp, model.vertices, model.vertex_count, color);
}

JC_Vertex jc_barycentric_interpolate(JC_Vertex v1, JC_Vertex v2, JC_Vertex v3,
        float alpha, float beta, float gamma)
{
    JC_Vec3 position;
    position.x = alpha*v1.position.x + beta*v2.position.x + gamma*v3.position.x;
    position.y = alpha*v1.position.y + beta*v2.position.y + gamma*v3.position.y;
    position.z = alpha*v1.position.z + beta*v2.position.z + gamma*v3.position.z;

    JC_Vec3 texcoord;
    texcoord.x = alpha*v1.texcoord.x + beta*v2.texcoord.x + gamma*v3.texcoord.x;
    texcoord.y = alpha*v1.texcoord.y + beta*v2.texcoord.y + gamma*v3.texcoord.y;
    texcoord.z = alpha*v1.texcoord.z + beta*v2.texcoord.z + gamma*v3.texcoord.z;

    JC_Vec3 normal;
    normal.x = alpha*v1.normal.x + beta*v2.normal.x + gamma*v3.normal.x;
    normal.y = alpha*v1.normal.y + beta*v2.normal.y + gamma*v3.normal.y;
    normal.z = alpha*v1.normal.z + beta*v2.normal.z + gamma*v3.normal.z;

    JC_Color color ;
    color.r = alpha*v1.color.r + beta*v2.color.r + gamma*v3.color.r;
    color.g = alpha*v1.color.g + beta*v2.color.g + gamma*v3.color.g;
    color.b = alpha*v1.color.b + beta*v2.color.b + gamma*v3.color.b;
    color.a = alpha*v1.color.a + beta*v2.color.a + gamma*v3.color.a;

    JC_Vertex result = { position, texcoord, normal, color };
    return result;
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
        (color >> 24) & 0xff,
        (color >> 16) & 0xff,
        (color >> 8)  & 0xff,
        (color >> 0)  & 0xff,
    };
    return c;
}

JC_Color jc_color_add(JC_Color ca, JC_Color cb)
{
    JC_Color c = {
        JC_MIN(255, ca.r + cb.r),
        JC_MIN(255, ca.g + cb.g),
        JC_MIN(255, ca.b + cb.b),
        JC_MIN(255, ca.a + cb.a),
    };
    return c;
}

// Integer promotion clutches up
JC_Color jc_color_mul(JC_Color ca, JC_Color cb)
{
    JC_Color c = {
        ca.r * cb.r / 255,
        ca.g * cb.g / 255,
        ca.b * cb.b / 255,
        ca.a * cb.a / 255,
    };
    return c;
}

JC_Color jc_color_scale(JC_Color a, float s)
{
    JC_Color c = { a.r * s, a.g * s, a.b * s, a.a * s };
    return c;
}

JC_Color jc_color_lerp(JC_Color a, JC_Color b, float t)
{
    JC_Color c = {
        (1.0f-t)*a.r + t*b.r,
        (1.0f-t)*a.g + t*b.g,
        (1.0f-t)*a.b + t*b.b,
        (1.0f-t)*a.a + t*b.a,
    };
    return c;
}


//===========================
// Linear Algebra stuff
//==========================
// Many of these are just modified from raylib

// Vector functions

JC_Vec4 jc_vec4_transform(JC_Matrix a, JC_Vec3 v) {
    JC_Vec4 u;
    u.x = a.m11 * v.x + a.m12 * v.y + a.m13 * v.z + a.m14;
    u.y = a.m21 * v.x + a.m22 * v.y + a.m23 * v.z + a.m24;
    u.z = a.m31 * v.x + a.m32 * v.y + a.m33 * v.z + a.m34;
    u.w = a.m41 * v.x + a.m42 * v.y + a.m43 * v.z + a.m44;
    return u;
}

JC_Vec4 jc_vec4_lerp(JC_Vec4 a, JC_Vec4 b, float t)
{
    JC_Vec4 v = {
        (1.0f - t)*a.x + t*b.x,
        (1.0f - t)*a.y + t*b.y,
        (1.0f - t)*a.z + t*b.z,
        (1.0f - t)*a.w + t*b.w,
    };
    return v;
}

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

JC_Vec3 jc_vec3_scale(JC_Vec3 v, float s)
{
    JC_Vec3 result = { v.x * s, v.y * s, v.z * s };
    return result;
}

JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v) {
    JC_Vec3 u;
    u.x = a.m11 * v.x + a.m12 * v.y + a.m13 * v.z + a.m14;
    u.y = a.m21 * v.x + a.m22 * v.y + a.m23 * v.z + a.m24;
    u.z = a.m31 * v.x + a.m32 * v.y + a.m33 * v.z + a.m34;
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

float jc_vec3_angle(JC_Vec3 a, JC_Vec3 b)
{
    float result = 0.0f;

    JC_Vec3 cross = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    float len = sqrtf(cross.x*cross.x + cross.y*cross.y + cross.z*cross.z);
    float dot = (a.x*b.x + a.y*b.y + a.z*b.z);
    result = atan2f(len, dot);

    return result;
}

JC_Vec3 jc_vec3_lerp(JC_Vec3 a, JC_Vec3 b, float t)
{
    JC_Vec3 v = {
        (1.0f - t)*a.x + t*b.x,
        (1.0f - t)*a.y + t*b.y,
        (1.0f - t)*a.z + t*b.z,
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

JC_Matrix jc_matrix_orthographic(float left, float right, float bottom, float top, float near, float far)
{
    JC_Matrix m = {0};
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    m.m11 = 2.0f / rl;
    m.m22 = 2.0f / tb;
    m.m33 = -2.0f / fn;

    m.m14 = -(right + left)/rl;
    m.m24 = -(top + bottom)/tb;
    m.m34 = -(far + near)/fn;
    m.m44 = 1.0f;
    return m;
}

JC_Matrix jc_matrix_perspective(float fov, float aspect, float near, float far)
{
    JC_Matrix m = {0};
    float top = near*tanf(fov*0.5f);
    float bottom = -top;
    float right = top*aspect;
    float left = -right;

    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    m.m11 = near*2.0f/rl;
    m.m22 = near*2.0f/tb;
    m.m13 = (right + left)/rl;
    m.m23 = (top + bottom)/tb;
    // non linear z
    m.m33 = -(far + near)/fn;
    m.m34 = -(2.0f*far*near)/fn;

    m.m43 = -1.0f; // w stores -z which we use to do perspective division
    return m;
}

JC_Matrix jc_matrix_viewport(float width, float height)
{
    JC_Matrix m = {0};
    m.m11 = width / 2.0f;
    m.m14 = width / 2.0f;
    m.m22 = height / 2.0f;
    m.m24 = height / 2.0f;

    m.m33 = -0.5f;
    m.m34 = 0.5f;
    m.m44 = 1.0f;
    return m;
}

JC_Matrix jc_matrix_view(JC_Camera camera)
{
    JC_Vec3 n = jc_vec3_normalize(jc_vec3_sub(camera.position, camera.target));
    JC_Vec3 l = jc_vec3_normalize(jc_vec3_cross(camera.up, n));
    JC_Vec3 m = jc_vec3_normalize(jc_vec3_cross(n, l));
    JC_Vec3 c = camera.position;

    JC_Matrix result = {
        l.x, l.y, l.z, 0,
        m.x, m.y, m.z, 0,
        n.x, n.y, n.z, 0,
        0,     0,   0, 1,
    };
    result = jc_matrix_mul(result, jc_matrix_translate(-c.x, -c.y, -c.z));
    return result;
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

JC_Matrix jc_matrix_scale(float x, float y, float z)
{
    JC_Matrix m = {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    return m;
}

JC_Matrix jc_matrix_rotate(JC_Vec3 axis, float angle)
{
    JC_Matrix result = {0};
    float x = axis.x, y = axis.y, z = axis.z;

    float length_sqr = x*x + y*y + z*z;
    if (length_sqr != 1.0f && length_sqr != 0.0f) {
        float inv_length = 1.0f/sqrtf(length_sqr);
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
    }

    float s = sinf(angle);
    float c = cosf(angle);
    float t = 1.0f - c;

    result.m11 = x*x*t + c;
    result.m12 = y*x*t + z*s;
    result.m13 = z*x*t - y*s;
    result.m14 = 0.0f;

    result.m21 = x*y*t - z*s;
    result.m22 = y*y*t + c;
    result.m23 = z*y*t + x*s;
    result.m24 = 0.0f;

    result.m31 = x*z*t + y*s;
    result.m32 = y*z*t - x*s;
    result.m33 = z*z*t + c;
    result.m34 = 0.0f;

    result.m41 = 0.0f;
    result.m42 = 0.0f;
    result.m43 = 0.0f;
    result.m44 = 1.0f;

    return result;
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
#undef da_append

#endif // JCANVAS_IMPLEMENTATION
//======================================
// Strip prefixes from types and functions. This idea is taken from https://github.com/tsoding/nob.h
#ifndef JC_PREFIX
#define Canvas JC_Canvas
#define Image JC_Image
#define Color JC_Color
#define Camera JC_Camera
#define Model JC_Model
#define Vertex JC_Vertex
#define Matrix JC_Matrix
#define Rect JC_Rect
#define Vec3 JC_Vec3
#define Vec2 JC_Vec2

#define PERSPECTIVE JC_PERSPECTIVE
#define ORTHOGRAPHIC JC_ORTHOGRAPHIC
#define FILTER_NEAREST JC_FILTER_NEREAST
#define FILTER_BILINEAR JC_FILTER_BILINEAR
#define WRAP_REPEAT JC_WRAP_REPEAT
#define WRAP_CLAMP JC_WRAP_CLAMP

#define canvas_create jc_canvas_create
#define image_create jc_canvas_create
#define canvas_destroy jc_canvas_destroy
#define image_destroy jc_canvas_destroy
#define subcanvas jc_subcanvas
#define subimage jc_subcanvas
#define canvas_resize jc_canvas_resize
#define image_resize jc_canvas_resize

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

#define begin_mode_3d jc_begin_mode_3d
#define end_mode_3d jc_end_mode_3d
#define disable_depth jc_disable_depth
#define enable_depth jc_enable_depth
#define set_shader jc_set_shader
#define unset_shader jc_unset_shader
#define set_shader_uniforms jc_set_shader_uniforms
#define image_sample jc_image_sample

#define rasterize_triangle jc_rasterize_triangle
#define render_geometry jc_render_geometry
#define render_geometry_ex jc_render_geometry_ex
#define render_geometry_lines jc_render_geometry_lines
#define draw_model jc_draw_model
#define draw_model_wires jc_draw_model_wires

#define barycentric_interpolate jc_barycentric_interpolate
#define color_from_int jc_color_from_int
#define color_blend_alpha jc_color_blend_alpha
#define color_add jc_color_add
#define color_mul jc_color_mul
#define color_scale jc_color_scale
#define color_lerp jc_color_lerp

#define DEG2RAD JC_DEG2RAD
#define RAD2DEG JC_RAD2DEG
#define MIN JC_MIN
#define MAX JC_MAX
#define ABS JC_ABS
#define SWAP JC_SWAP

#define orthographic jc_orthographic
#define perspective jc_perspective
#define vec3_add jc_vec3_add
#define vec3_sub jc_vec3_sub
#define vec3_scale jc_vec3_scale
#define vec3_transform jc_vec3_transform
#define vec4_transform jc_vec4_transform
#define vec3_normalize jc_vec3_normalize
#define vec3_cross jc_vec3_cross
#define vec3_dot jc_vec3_dot
#define vec3_angle jc_vec3_angle
#define vec3_length jc_vec3_length

#define matrix_identity jc_matrix_identity
#define matrix_view jc_matrix_view
#define matrix_translate jc_matrix_translate
#define matrix_scale jc_matrix_scale
#define matrix_rotate jc_matrix_rotate
#define matrix_rotate_x jc_matrix_rotate_x
#define matrix_rotate_y jc_matrix_rotate_y
#define matrix_rotate_z jc_matrix_rotate_z
#define matrix_add jc_matrix_add
#define matrix_sub jc_matrix_sub
#define matrix_mul jc_matrix_mul
#endif // JC_PREFIX

#endif // JCANVAS_H
