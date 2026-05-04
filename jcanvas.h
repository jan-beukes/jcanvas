// jcanvas is a simple software rendering library for drawing into a pixel buffer
// Most of the rendering techniques were learned from the amazing series https://haqr.eu/tinyrenderer
// Raylib source code was also used as a reference for many different things https://github.com/raysan5/raylib
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

// TODO: Customizable math and alloc functions
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

// NOTE: This is taken from raylib 
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
    // see JC_Filter and JC_Wrap
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
    JC_Vec4 color;
} JC_Vertex;

typedef struct {
    float x, y;
    float w, h;
} JC_Rect;

// You can change these
// TODO: maybe use runtime values that can be set with jc_set_near_plane or something
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
// TODO: support models with multiple meshes/materials by allocating a mesh per material used
typedef struct {
    JC_Vertex *vertices;
    int vertex_count;
    JC_Matrix transform;

    // This is a simple diffuse texture that will be used when no shader is set
    JC_Image texture;
} JC_Model;

typedef struct {
    JC_Vertex v1, v2, v3;
    JC_Vec3 p1, p2, p3;
    JC_Image texture;
} JC_Triangle;

// This is a "shader" that is called for every pixel during rasterization
// input will hold the interpolated values of the vertices given to the rasterize function
// It should return TRUE if the pixel was set or FALSE to discard
typedef bool (*JC_ShaderFunc)(JC_Vec4 *out, JC_Vertex input, JC_Vec3 pos, void *uniforms);

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
void jc_canvas_resize(JC_Canvas *canvas, int w, int h);
void jc_canvas_destroy(JC_Canvas *canvas);

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

//=====2D drawing=====
// These functions are used to draw another canvas/image to a canvas
void jc_blit(JC_Canvas canvas, JC_Image image, int x, int y);
// Like cv_blit but it scales the image so that it occupies the rectangle (x, y, w, h)
void jc_blit_rect(JC_Canvas canvas, JC_Image image, JC_Rect dst, JC_Rect src);

// TODO: vector and rect argument functions?
void jc_fill(JC_Canvas canvas, JC_Color color);
void jc_draw_rect(JC_Canvas canvas, int x, int y, int w, int h, JC_Color color);
void jc_draw_line(JC_Canvas canvas, int ax, int ay, int bx, int by, JC_Color color);
void jc_draw_triangle(JC_Canvas canvas, int ax, int ay, int bx, int by, int cx, int cy, JC_Color color);

// This sets the canvas, camera and zbuffer to be used for 3d rendering functions 
// it also clears the zbuffer
void jc_begin_mode_3d(JC_Canvas canvas, JC_Camera camera, float *zbuffer);
void jc_end_mode_3d(void);

// disable backface culling when rasterizing triangles
void jc_disable_backface_culling(void);
void jc_enable_backface_culling(void);

// If you want to disable any writes to the zbuffer
void jc_disable_depth(void);
void jc_enable_depth(void);

// The shader func is only called in the rasterize function
void jc_set_shader(JC_ShaderFunc func, void *uniforms);
void jc_set_shader_uniforms(void *uniforms);
void jc_unset_shader(void);

// Samples an image at the given uv coordinate. UV coordinates have y up so this function will flip y to
// read the correct value from the image
JC_Vec4 jc_image_sample(JC_Image image, float u, float v);

// This is the main rasterization function called to draw triangles. Y will be up so this function flips y.
void jc_rasterize_triangle(JC_Canvas canvas, float *zbuffer, JC_Triangle triangle);

//=====3D drawing=====
// These are the function used to dispatch a render. They are used by all the 3d drawing functions
// The zbuffer is what is used for depth testing so make sure to use the same one for the entire scene
// It should be of size width*height and initialized and cleared to zero (after projection z is in [1, 0]).
void jc_render_geometry(JC_Canvas canvas, JC_Vertex *vertices, int vertex_count, JC_Matrix model, JC_Matrix view_proj,
        JC_Image texture, JC_Color color, float *zbuffer);
void jc_render_geometry_lines(JC_Canvas canvas, JC_Vertex *vertices, int vertex_count,
        JC_Matrix model, JC_Matrix view_proj, JC_Color color, float *zbuffer);

void jc_draw_model(JC_Model model, JC_Vec3 position, JC_Color color);
void jc_draw_model_wires(JC_Model model, JC_Vec3 position, JC_Color color);

void jc_draw_cube(JC_Vec3 position, JC_Vec3 scale, JC_Color color);
void jc_draw_cube_wires(JC_Vec3 position, JC_Vec3 scale, JC_Color color);
void jc_draw_sphere(JC_Vec3 position, float radius, JC_Color color);
void jc_draw_sphere_wires(JC_Vec3 position, float radius, JC_Color color);

inline double jc_signed_triangle_area(float ax, float ay, float bx, float by, float cx, float cy);
inline int jc_clip_triangle(JC_Vertex v[6], JC_Vec4 clip[6]);
inline JC_Vertex jc_vertex_to_ndc(JC_Vertex v, JC_Vec4 clip);
inline JC_Vertex jc_barycentric_interpolate(JC_Vertex v1, JC_Vertex v2, JC_Vertex v3, float alpha, float beta, float gamma);
inline JC_Vertex jc_vertex_lerp(JC_Vertex a, JC_Vertex b, float t);

//======Color=========
// the integer is assumed RGBA with R being the big end
inline JC_Color jc_color_from_int(uint32_t color);
inline JC_Color jc_colorb(JC_Vec4 v);
inline JC_Color jc_color_blend_alpha(JC_Color a, JC_Color b);
inline JC_Color jc_color_add(JC_Color a, JC_Color b);
inline JC_Color jc_color_mul(JC_Color a, JC_Color b);
inline JC_Color jc_color_scale(JC_Color a, float s);
inline JC_Color jc_color_lerp(JC_Color a, JC_Color b, float t);

//===Math Functions===
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

inline JC_Vec4 jc_vec4_add(JC_Vec4 a, JC_Vec4 b);
inline JC_Vec4 jc_vec4_sub(JC_Vec4 a, JC_Vec4 b);
inline JC_Vec4 jc_vec4_mul(JC_Vec4 a, JC_Vec4 b);
inline JC_Vec4 jc_vec4_scale(JC_Vec4 a, float s);
inline JC_Vec4 jc_colorf(JC_Color c);
inline JC_Vec4 jc_vec4_transform(JC_Matrix a, JC_Vec3 v);
inline JC_Vec4 jc_vec4_lerp(JC_Vec4 a, JC_Vec4 b, float t);

inline JC_Vec3 jc_vec3_add(JC_Vec3 a, JC_Vec3 b);
inline JC_Vec3 jc_vec3_sub(JC_Vec3 a, JC_Vec3 b);
inline JC_Vec3 jc_vec3_scale(JC_Vec3 v, float s);
inline JC_Vec3 jc_vec3_transform(JC_Matrix a, JC_Vec3 v);
inline JC_Vec3 jc_vec3_normalize(JC_Vec3 v);
inline JC_Vec3 jc_vec3_cross(JC_Vec3 a, JC_Vec3 b);
inline JC_Vec3 jc_vec3_lerp(JC_Vec3 a, JC_Vec3 b, float t);
inline float jc_vec3_dot(JC_Vec3 a, JC_Vec3 b);
inline float jc_vec3_length(JC_Vec3 v);
inline float jc_vec3_angle(JC_Vec3 a, JC_Vec3 b);

// TODO: Vector2 functions

inline JC_Matrix jc_matrix_identity(void);
inline JC_Matrix jc_matrix_orthographic(float left, float right, float bottom, float top, float near, float far);
inline JC_Matrix jc_matrix_perspective(float fov, float aspect, float near, float far);
inline JC_Matrix jc_matrix_viewport(float width, float height);
inline JC_Matrix jc_matrix_view(JC_Camera camera);

inline JC_Matrix jc_matrix_translate(float x, float y, float z);
inline JC_Matrix jc_matrix_scale(float x, float y, float z);
inline JC_Matrix jc_matrix_rotate(JC_Vec3 axis, float angle);
inline JC_Matrix jc_matrix_rotate_x(float angle);
inline JC_Matrix jc_matrix_rotate_y(float angle);
inline JC_Matrix jc_matrix_rotate_z(float angle);

inline JC_Matrix jc_matrix_add(JC_Matrix a, JC_Matrix b);
inline JC_Matrix jc_matrix_sub(JC_Matrix a, JC_Matrix b);
inline JC_Matrix jc_matrix_mul(JC_Matrix a, JC_Matrix b);
inline JC_Matrix jc_matrix_transpose(JC_Matrix a);
inline JC_Matrix jc_matrix_inv(JC_Matrix a);

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
    bool backface_culling_disabled; 
    JC_Canvas canvas;
    JC_Matrix view_proj; // this stored instead of camera
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

void jc_print_vec3(JC_Vec3 v)
{
    printf("[%.3f %.3f %.3f]\n", v.x, v.y, v.z);
}

void jc_print_matrix(JC_Matrix m)
{
    printf("[%.2f %.2f %.2f %.2f]\n[%.2f %.2f %.2f %.2f]\n[%.2f %.2f %.2f %.2f]\n[%.2f %.2f %.2f %.2f]\n\n",
            m.m11, m.m12, m.m13, m.m14,
            m.m21, m.m22, m.m23, m.m24,
            m.m31, m.m32, m.m33, m.m34,
            m.m41, m.m42, m.m43, m.m44);
}

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

static char *_read_entire_file(const char *path, long *size)
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
    *image = CLITERAL(JC_Canvas){0};

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
            vertex.color = jc_colorf(WHITE);
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

// TODO: Texture filtering
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

    double inv_area = 1.0 / total_area;
    for (int y = bb_miny; y <= bb_maxy; y++) {
        for (int x = bb_minx; x <= bb_maxx; x++) {
            // The barycentric coord is proportional to the area of the triangle
            // made from P with the other two vertices. When the point is close
            // to a vertex the area made with the other vertices will be larger
            double alpha = jc_signed_triangle_area(x, y, bx, by, cx, cy) * inv_area;
            double beta  = jc_signed_triangle_area(x, y, cx, cy, ax, ay) * inv_area;
            double gamma = 1.0 - alpha - beta;
            // A negative coordinate means that the point is not in the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            if (!JC_IN_BOUNDS(canvas, x, y)) continue;
            JC_PIXEL(canvas, x, y) = color;
        }
    }
}

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

void jc_disable_depth(void) { _jc_state.depth_disabled = true; }
void jc_enable_depth(void) { _jc_state.depth_disabled = false; }
void jc_disable_backface_culling(void) { _jc_state.backface_culling_disabled = true; }
void jc_enable_backface_culling(void) { _jc_state.backface_culling_disabled = false; }

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

JC_Vec4 jc_image_sample(JC_Image image, float u, float v)
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
            assert(false && "Invalid wrap");
    }
    JC_Color out = JC_PIXEL(image, x, y);
    return jc_colorf(out);
}

void jc_rasterize_line(JC_Canvas canvas, float *zbuffer, JC_Vec3 p1, JC_Vec3 p2, JC_Color color)
{
    // if steep we swap x and y to iterate over y
    bool steep = JC_ABS(p1.x - p2.x) < JC_ABS(p1.y - p2.y);
    if (steep) {
        JC_SWAP(p1.x, p1.y);
        JC_SWAP(p2.x, p2.y);
    }
    // make sure that ax is the smallest of the points
    if (p1.x > p2.x) {
        JC_SWAP(p1, p2);
    }

    float y = p1.y;
    float z = p1.z;
    float dy = (float)(p2.y-p1.y) / (p2.x-p1.x);
    float dz = (float)(p2.z-p1.z) / (p2.x-p1.x);
    for (int x = p1.x; x < (int)p2.x; ++x) {
        // since y is actually x in this case
        int yi = (int)y;
        if (steep) {
            if (JC_IN_BOUNDS(canvas, yi, x) && z > zbuffer[x*canvas.width + yi]) {
                JC_PIXEL(canvas, yi, (canvas.height-1) - x) = color;
                if (!_jc_state.depth_disabled)
                    zbuffer[x*canvas.width + yi] = z;
            }
        } else {
            if (JC_IN_BOUNDS(canvas, x, yi) && z > zbuffer[yi*canvas.width + x]) {
                JC_PIXEL(canvas, x, (canvas.height-1) - yi) = color;
                if (!_jc_state.depth_disabled)
                    zbuffer[yi*canvas.width + x] = z;
            }
        }
        // accumulate dy
        y += dy;
        z += dz;
    }
}

// TODO: Use a VertexAttribute structure so that I can add more stuff (like the world positions)?
void jc_rasterize_triangle(JC_Canvas canvas, float *zbuffer, JC_Triangle triangle)
{
    JC_Vertex v1 = triangle.v1, v2 = triangle.v2, v3 = triangle.v3;
    JC_Image texture = triangle.texture;

    float ax = v1.position.x, ay = v1.position.y;
    float bx = v2.position.x, by = v2.position.y;
    float cx = v3.position.x, cy = v3.position.y;

    int min_axbx = JC_MIN(ax, bx);
    int bb_minx = JC_MIN(min_axbx, cx);
    int min_ayby = JC_MIN(ay, by);
    int bb_miny  = JC_MIN(min_ayby, cy);
    JC_CLAMP(canvas, bb_minx, bb_miny);

    int max_axbx = JC_MAX(ax, bx);
    int bb_maxx  = JC_MAX(max_axbx, cx);
    int max_ayby = JC_MAX(ay, by);
    int bb_maxy  = JC_MAX(max_ayby, cy);
    JC_CLAMP(canvas, bb_maxx, bb_maxy);

    double total_area = jc_signed_triangle_area(ax, ay, bx, by, cx, cy);
    bool is_backface = total_area < 0.05;
    if (!_jc_state.backface_culling_disabled && is_backface) return;
    bool has_texture = texture.pixels != NULL;

    double inv_area = 1.0 / total_area;
    for (int y = bb_miny; y <= bb_maxy; y++) {
        float *zptr = &zbuffer[y*canvas.width + bb_minx];
        for (int x = bb_minx; x <= bb_maxx; x++, zptr++) {
            // The barycentric coord is proportional to the area of the triangle
            // made from P with the other two vertices. When the point is close
            // to a vertex the area made with the other vertices will be larger
            double alpha = jc_signed_triangle_area(x, y, bx, by, cx, cy) * inv_area;
            double beta  = jc_signed_triangle_area(x, y, cx, cy, ax, ay) * inv_area;
            double gamma = 1.0 - alpha - beta;
            // A negative coordinate means that the point is not in the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            float z = alpha*v1.position.z + beta*v2.position.z + gamma*v3.position.z;
            if (z >= 1.0f || z <= *zptr) continue;

            JC_Vertex frag = jc_barycentric_interpolate(v1, v2, v3, alpha, beta, gamma);
            if (is_backface) frag.normal = jc_vec3_scale(frag.normal, -1);

            // get back u, v from perspective corrected space by dividing by 1 / z
            float w = 1.0f / frag.texcoord.z;
            frag.texcoord.x *= w, frag.texcoord.y *= w;
            frag.color = jc_vec4_scale(frag.color, w);

            JC_Vec4 out = frag.color;
            bool discard = false;
            if (_jc_state.active_shader != NULL) {
                // Interpolate world pos. Should do this in barycentric function if I want to add more
                // attributes
                JC_Vec3 pos;
                pos.x = triangle.p1.x*alpha + triangle.p2.x*beta + triangle.p3.x*gamma;
                pos.y = triangle.p1.y*alpha + triangle.p2.y*beta + triangle.p3.y*gamma;
                pos.z = triangle.p1.z*alpha + triangle.p2.z*beta + triangle.p3.z*gamma;
                pos = jc_vec3_scale(pos, w);
                discard = !_jc_state.active_shader(&out, frag, pos, _jc_state.active_shader_uniforms);
            } else if (has_texture) {
                JC_Vec4 texcolor = jc_image_sample(texture, frag.texcoord.x, frag.texcoord.y);
                out = jc_vec4_mul(out, texcolor);
            }

            if (!discard) {
                if (!_jc_state.depth_disabled) *zptr = z;
                // NOTE: here we flip to the screen
                JC_PIXEL(canvas, x, (canvas.height-1) - y) = jc_colorb(out);
            }
        }
    }
}

void jc_render_geometry(JC_Canvas canvas, JC_Vertex *vertices, int vertex_count, JC_Matrix model,
        JC_Matrix view_proj, JC_Image texture, JC_Color color, float *zbuffer)
{
    JC_Matrix viewport = jc_matrix_viewport(canvas.width, canvas.height);
    JC_Matrix mvp = jc_matrix_mul(view_proj, model);
    int triangle_count = vertex_count / 3;
    for (int i = 0; i < triangle_count; i++) {
        JC_Vertex v[6]  = {0};
        JC_Vec3 pos[6]  = {0};
        JC_Vec4 clip[6] = {0};

        // transform to clip space
        for (int j = 0; j < 3; j++) {
            v[j] = vertices[3*i + j];
            clip[j] = jc_vec4_transform(mvp, v[j].position);
            v[j].color = jc_vec4_mul(v[j].color, jc_colorf(color));
        }

        int triangle_count = jc_clip_triangle(v, clip);
        for (int tri = 0; tri < triangle_count; tri++) {
            for (int j = 0; j < 3; j++) {
                int idx = tri*3 + j;
                pos[idx] = jc_vec3_transform(model, v[idx].position);
                // transform before sending off to rasterization
                // perspective division
                float w_inv = 1.0f / clip[idx].w;
                v[idx] = jc_vertex_to_ndc(v[idx], clip[idx]);
                pos[idx].x *= w_inv, pos[idx].y *= w_inv, pos[idx].z *= w_inv;

                // transform normals
                JC_Matrix mat_norm = model;
                mat_norm.m14 = 0, mat_norm.m24 = 0, mat_norm.m34 = 0;
                mat_norm = jc_matrix_transpose(jc_matrix_inv(mat_norm));
                v[idx].normal = jc_vec3_normalize(jc_vec3_transform(mat_norm, v[idx].normal));
                v[idx].position = jc_vec3_transform(viewport, v[idx].position);
            }
            JC_Triangle triangle = {
                v[3*tri],   v[3*tri+1],   v[3*tri+2],
                pos[3*tri], pos[3*tri+1], pos[3*tri+2],
                texture
            };
            jc_rasterize_triangle(canvas, zbuffer, triangle);
        }
    }
}

void jc_render_geometry_lines(JC_Canvas canvas, JC_Vertex *vertices, int vertex_count, JC_Matrix model,
        JC_Matrix view_proj, JC_Color color, float *zbuffer)
{
    JC_Matrix viewport = jc_matrix_viewport(canvas.width, canvas.height);
    int triangle_count = vertex_count / 3;
    JC_Matrix mvp = jc_matrix_mul(view_proj, model);
    for (int i = 0; i < triangle_count; i++) {
        JC_Vertex v[6] = {0};
        JC_Vec4 clip[6] = {0};

        v[0] = vertices[3*i];
        v[1] = vertices[3*i + 1];
        v[2] = vertices[3*i + 2];
        // transform to clip space
        for (int j = 0; j < 3; j++) {
            clip[j] = jc_vec4_transform(mvp, v[j].position);
        }
        int triangle_count = jc_clip_triangle(v, clip);
        for (int tri = 0; tri < triangle_count; tri++) {
            for (int j = 0; j < 3; j++) {
                int idx = tri*3 + j;
                float inv_w = 1.0f / clip[idx].w;
                v[idx].position.x = clip[idx].x * inv_w; 
                v[idx].position.y = clip[idx].y * inv_w; 
                v[idx].position.z = clip[idx].z * inv_w; 
                v[idx].position = jc_vec3_transform(viewport, v[idx].position);
            }
            JC_Vec3 p1 = v[3*tri].position, p2 = v[3*tri+1].position, p3 = v[3*tri+2].position;
            jc_rasterize_line(canvas, zbuffer, p1, p2, color);
            if (triangle_count == 1 || tri == 1) jc_rasterize_line(canvas, zbuffer, p2, p3, color);
            if (triangle_count == 1 || tri == 0) jc_rasterize_line(canvas, zbuffer, p3, p1, color);
        }

    }
}

void jc_draw_model(JC_Model model, JC_Vec3 position, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;

    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix mat_model = jc_matrix_mul(translate, model.transform);
    jc_render_geometry(canvas, model.vertices, model.vertex_count, mat_model, view_proj,
            model.texture, color, zbuffer);
}

// NOTE: This does not use any of the vertex colors or the model texture
void jc_draw_model_wires(JC_Model model, JC_Vec3 position, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;

    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix mat_model = jc_matrix_mul(translate, model.transform);
    jc_render_geometry_lines(canvas, model.vertices, model.vertex_count, mat_model, view_proj, color, zbuffer);
}

void jc_set_cube_vertices(JC_Vertex *v)
{
    JC_Vec3 n = {0};
    JC_Vec3 t = {0};
    JC_Vec4 c = jc_colorf(WHITE);
    // front face
    n = CLITERAL(JC_Vec3){ 0, 0, 1 };
    v[0] = CLITERAL(JC_Vertex){ { -0.5,  0.5, 0.5 }, t, n, c };
    v[1] = CLITERAL(JC_Vertex){ {  0.5, -0.5, 0.5 }, t, n, c };
    v[2] = CLITERAL(JC_Vertex){ {  0.5,  0.5, 0.5 }, t, n, c };
    v[3] = CLITERAL(JC_Vertex){ { -0.5,  0.5, 0.5 }, t, n, c };
    v[4] = CLITERAL(JC_Vertex){ { -0.5, -0.5, 0.5 }, t, n, c };
    v[5] = CLITERAL(JC_Vertex){ {  0.5, -0.5, 0.5 }, t, n, c };

    // back face
    n = CLITERAL(JC_Vec3){ 0, 0, -1 };
    v[6]  = CLITERAL(JC_Vertex){ {  0.5,  0.5, -0.5 }, t, n, c };
    v[7]  = CLITERAL(JC_Vertex){ { -0.5, -0.5, -0.5 }, t, n, c };
    v[8]  = CLITERAL(JC_Vertex){ { -0.5,  0.5, -0.5 }, t, n, c };
    v[9]  = CLITERAL(JC_Vertex){ {  0.5,  0.5, -0.5 }, t, n, c };
    v[10] = CLITERAL(JC_Vertex){ {  0.5, -0.5, -0.5 }, t, n, c };
    v[11] = CLITERAL(JC_Vertex){ { -0.5, -0.5, -0.5 }, t, n, c };

    // left face
    n = CLITERAL(JC_Vec3){ -1, 0, 0 };
    v[12] = CLITERAL(JC_Vertex){ { -0.5,  0.5, -0.5 }, t, n, c };
    v[13] = CLITERAL(JC_Vertex){ { -0.5, -0.5,  0.5 }, t, n, c };
    v[14] = CLITERAL(JC_Vertex){ { -0.5,  0.5,  0.5 }, t, n, c };
    v[15] = CLITERAL(JC_Vertex){ { -0.5,  0.5, -0.5 }, t, n, c };
    v[16] = CLITERAL(JC_Vertex){ { -0.5, -0.5, -0.5 }, t, n, c };
    v[17] = CLITERAL(JC_Vertex){ { -0.5, -0.5,  0.5 }, t, n, c };

    // right face
    n = CLITERAL(JC_Vec3){ 1, 0, 0 };
    v[18] = CLITERAL(JC_Vertex){ {  0.5,  0.5,  0.5 }, t, n, c };
    v[19] = CLITERAL(JC_Vertex){ {  0.5, -0.5, -0.5 }, t, n, c };
    v[20] = CLITERAL(JC_Vertex){ {  0.5,  0.5, -0.5 }, t, n, c };
    v[21] = CLITERAL(JC_Vertex){ {  0.5,  0.5,  0.5 }, t, n, c };
    v[22] = CLITERAL(JC_Vertex){ {  0.5, -0.5,  0.5 }, t, n, c };
    v[23] = CLITERAL(JC_Vertex){ {  0.5, -0.5, -0.5 }, t, n, c };

    // top face
    n = CLITERAL(JC_Vec3){ 0, 1, 0 };
    v[24] = CLITERAL(JC_Vertex){ {  0.5,  0.5,  0.5 }, t, n, c };
    v[25] = CLITERAL(JC_Vertex){ { -0.5,  0.5, -0.5 }, t, n, c };
    v[26] = CLITERAL(JC_Vertex){ { -0.5,  0.5,  0.5 }, t, n, c };
    v[27] = CLITERAL(JC_Vertex){ {  0.5,  0.5,  0.5 }, t, n, c };
    v[28] = CLITERAL(JC_Vertex){ {  0.5,  0.5, -0.5 }, t, n, c };
    v[29] = CLITERAL(JC_Vertex){ { -0.5,  0.5, -0.5 }, t, n, c };

    // bottom face
    n = CLITERAL(JC_Vec3){ 0, 1, 0 };
    v[30] = CLITERAL(JC_Vertex){ { -0.5, -0.5,  0.5 }, t, n, c };
    v[31] = CLITERAL(JC_Vertex){ {  0.5, -0.5, -0.5 }, t, n, c };
    v[32] = CLITERAL(JC_Vertex){ {  0.5, -0.5,  0.5 }, t, n, c };
    v[33] = CLITERAL(JC_Vertex){ { -0.5, -0.5,  0.5 }, t, n, c };
    v[34] = CLITERAL(JC_Vertex){ { -0.5, -0.5, -0.5 }, t, n, c };
    v[35] = CLITERAL(JC_Vertex){ {  0.5, -0.5, -0.5 }, t, n, c };

}

void jc_draw_cube(JC_Vec3 position, JC_Vec3 scale, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Vertex v[6*6];
    jc_set_cube_vertices(v);

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;
    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix model = jc_matrix_mul(translate, jc_matrix_scale(scale.x, scale.y, scale.z));

    JC_Image texture = {0};
    jc_render_geometry(canvas, v, 36, model, view_proj, texture, color, zbuffer);
}

void jc_draw_cube_wires(JC_Vec3 position, JC_Vec3 scale, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Vertex v[6*6];
    jc_set_cube_vertices(v);

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;
    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix model = jc_matrix_mul(translate, jc_matrix_scale(scale.x, scale.y, scale.z));
    jc_render_geometry_lines(canvas, v, 36, model, view_proj, color, zbuffer);
}

// NOTE: v must have space for slices rings*slices*6
void jc_set_sphere_vertices(JC_Vertex *v, int rings, int slices)
{
    float rangle = M_PI / rings;
    float sangle = (2.0f*M_PI) / slices;
    float cosring = cosf(rangle);
    float sinring = sinf(rangle);
    float cosslice = cosf(sangle);
    float sinslice = sinf(sangle);

    JC_Vec3 t = {0};
    JC_Vec4 c = jc_colorf(WHITE);
    JC_Vec3 p[4] = {0};
    p[2] = CLITERAL(JC_Vec3){ 0, 1, 0 };
    p[3] = CLITERAL(JC_Vec3){ sinring, cosring, 0 };
    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < slices; j++) {
            // Set new p0,p1 to old p2,p3
            p[0] = p[2];
            p[1] = p[3];
            // rotate around y
            p[2] = CLITERAL(JC_Vec3){ cosslice*p[2].x - sinslice*p[2].z, p[2].y,
                sinslice*p[2].x + cosslice*p[2].z };
            p[3] = CLITERAL(JC_Vec3){ cosslice*p[3].x - sinslice*p[3].z, p[3].y,
                sinslice*p[3].x + cosslice*p[3].z };

            int idx = (i*slices + j)*6;
            v[idx+0] = CLITERAL(JC_Vertex){ p[0], t, p[0], c };
            v[idx+1] = CLITERAL(JC_Vertex){ p[3], t, p[3], c };
            v[idx+2] = CLITERAL(JC_Vertex){ p[1], t, p[1], c };

            v[idx+3] = CLITERAL(JC_Vertex){ p[0], t, p[0], c };
            v[idx+4] = CLITERAL(JC_Vertex){ p[2], t, p[2], c };
            v[idx+5] = CLITERAL(JC_Vertex){ p[3], t, p[3], c };
        }
        // Rotate around z
        p[2] = p[3];
        p[3] = CLITERAL(JC_Vec3){ cosring*p[3].x + sinring*p[3].y,
            -sinring*p[3].x + cosring*p[3].y, p[3].z };
    }
}

#define SLICES 14
#define RINGS 15
void jc_draw_sphere(JC_Vec3 position, float radius, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Vertex v[RINGS*SLICES*6] = {0};
    jc_set_sphere_vertices(v, RINGS, SLICES);

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;
    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix model = jc_matrix_mul(translate, jc_matrix_scale(radius, radius, radius));

    JC_Image texture = {0};
    jc_render_geometry(canvas, v, RINGS*SLICES*6, model, view_proj, texture, color, zbuffer);
}

void jc_draw_sphere_wires(JC_Vec3 position, float radius, JC_Color color)
{
    if (!_jc_state.mode_3d_active) return;

    JC_Vertex v[RINGS*SLICES*6] = {0};
    jc_set_sphere_vertices(v, RINGS, SLICES);

    JC_Canvas canvas = _jc_state.canvas;
    float *zbuffer = _jc_state.zbuffer;
    JC_Matrix view_proj = _jc_state.view_proj;
    JC_Matrix translate = jc_matrix_translate(position.x, position.y, position.z);
    JC_Matrix model = jc_matrix_mul(translate, jc_matrix_scale(radius, radius, radius));
    jc_render_geometry_lines(canvas, v, RINGS*SLICES*6, model, view_proj, color, zbuffer);

}

inline JC_Vertex jc_vertex_lerp(JC_Vertex a, JC_Vertex b, float t)
{
    JC_Vertex v;
    v.position.x = a.position.x + t * (b.position.x - a.position.x);
    v.position.y = a.position.y + t * (b.position.y - a.position.y);
    v.position.z = a.position.z + t * (b.position.z - a.position.z);

    v.texcoord.x = a.texcoord.x + t * (b.texcoord.x - a.texcoord.x);
    v.texcoord.y = a.texcoord.y + t * (b.texcoord.y - a.texcoord.y);
    v.texcoord.z = a.texcoord.z + t * (b.texcoord.z - a.texcoord.z);

    v.normal.x = a.normal.x + t * (b.normal.x - a.normal.x);
    v.normal.y = a.normal.y + t * (b.normal.y - a.normal.y);
    v.normal.z = a.normal.z + t * (b.normal.z - a.normal.z);

    v.color.x = a.color.x + t * (b.color.x - a.color.x);
    v.color.y = a.color.y + t * (b.color.y - a.color.y);
    v.color.z = a.color.z + t * (b.color.z - a.color.z);
    v.color.w = a.color.w + t * (b.color.w - a.color.w);
    return v;
}

// Both arrays must be of size 6 for triangle splitting
// Returns the number of triangles to render
int jc_clip_triangle(JC_Vertex v[6], JC_Vec4 clip[6])
{
    int clipped[3] = {0};
    int clipped_count = 0;
    int unclipped[3] = {0};
    int unclipped_count = 0;
    for (int i = 0; i < 3; i++) {
        if (clip[i].w < JC_NEAR_PLANE) {
            clipped[clipped_count++] = i;
        } else {
            unclipped[unclipped_count++] = i;
        }
    }
    if (clipped_count == 0) return 1;
    else if (clipped_count == 3) return 0;

    if (clipped_count == 1) {
        int ci = clipped[0];
        int bi = (ci + 1) % 3;
        int ai = (ci + 2) % 3;

        JC_Vec4 a = clip[ai];
        JC_Vec4 b = clip[bi];
        JC_Vec4 c = clip[ci];

        // c to a
        double t1 = (JC_NEAR_PLANE - c.w) / (a.w - c.w);
        JC_Vec4 q1 = jc_vec4_lerp(c, a, t1);
        JC_Vertex vq1 = jc_vertex_lerp(v[ci], v[ai], t1);

        // c to b
        float t2 = (JC_NEAR_PLANE - c.w) / (b.w - c.w);
        JC_Vec4 q2 = jc_vec4_lerp(c, b, t2);
        JC_Vertex vq2 = jc_vertex_lerp(v[ci], v[bi], t2);

        // We need to store copy since we are writing to the vertices and don'y know which index we
        // will overwrite
        JC_Vertex vb = v[bi], va = v[ai];
        JC_Vec4 cb = clip[bi], ca = clip[ai];

        v[0] = va; clip[0] = ca;
        v[1] = vq1;   clip[1] = q1;
        v[2] = vb;    clip[2] = cb;

        v[3] = vq1;   clip[3] = q1;
        v[4] = vq2;   clip[4] = q2;
        v[5] = vb;    clip[5] = cb;

        return 2;
    } else if (clipped_count == 2) {

        int ai = unclipped[0];
        int bi = (ai + 1) % 3;
        int ci = (ai + 2) % 3;
        JC_Vec4 a = clip[ai];
        JC_Vec4 b = clip[bi];
        JC_Vec4 c = clip[ci];

        // b to a
        float t1 = (JC_NEAR_PLANE - b.w) / (a.w - b.w);
        JC_Vec4 q1 = jc_vec4_lerp(b, a, t1);
        JC_Vertex vq1 = jc_vertex_lerp(v[bi], v[ai], t1);

        // c to a
        float t2 = (JC_NEAR_PLANE - c.w) / (a.w - c.w);
        JC_Vec4 q2 = jc_vec4_lerp(c, a, t2);
        JC_Vertex vq2 = jc_vertex_lerp(v[ci], v[ai], t2);

        v[0] = v[ai]; clip[0] = clip[ai];
        v[1] = vq1;   clip[1] = q1;
        v[2] = vq2;   clip[2] = q2;
        return 1;
    }

    return 0;
}

JC_Vertex jc_vertex_to_ndc(JC_Vertex v, JC_Vec4 clip)
{
    float w_inv = 1.0f / clip.w;
    JC_Vertex result = v;
    result.position.x = clip.x * w_inv;
    result.position.y = clip.y * w_inv;
    result.position.z = clip.z * w_inv;
    // make sure that when we interpolate it in perspective corrected space
    result.texcoord.x *= w_inv;
    result.texcoord.y *= w_inv;
    // texcoord.z will store 1/z which we can then multiply with during rasterization
    result.texcoord.z = w_inv;

    result.color.x *= w_inv;
    result.color.y *= w_inv;
    result.color.z *= w_inv;
    result.color.w *= w_inv;

    return result;
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

    JC_Vec4 color;
    color.x = alpha*v1.color.x + beta*v2.color.x + gamma*v3.color.x;
    color.y = alpha*v1.color.y + beta*v2.color.y + gamma*v3.color.y;
    color.z = alpha*v1.color.z + beta*v2.color.z + gamma*v3.color.z;
    color.w = alpha*v1.color.w + beta*v2.color.w + gamma*v3.color.w;

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

JC_Color jc_colorb(JC_Vec4 v)
{
    JC_Color c = {
        v.x > 1.0f ? 255 : (v.x < 0.0f ? 0 : v.x * 255),
        v.y > 1.0f ? 255 : (v.y < 0.0f ? 0 : v.y * 255),
        v.z > 1.0f ? 255 : (v.z < 0.0f ? 0 : v.z * 255),
        v.w > 1.0f ? 255 : (v.w < 0.0f ? 0 : v.w * 255),
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
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a),
    };
    return c;
}


//===========================
// Linear Algebra stuff
//==========================
// Many of these are just modified from raymath

// Vector functions
JC_Vec4 jc_vec4_add(JC_Vec4 a, JC_Vec4 b)
{
    JC_Vec4 v = { a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w };
    return v;
}

JC_Vec4 jc_vec4_sub(JC_Vec4 a, JC_Vec4 b)
{
    JC_Vec4 v = { a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w };
    return v;
}

JC_Vec4 jc_vec4_mul(JC_Vec4 a, JC_Vec4 b)
{
    JC_Vec4 v = { a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w };
    return v;
}

JC_Vec4 jc_vec4_scale(JC_Vec4 a, float s)
{
    JC_Vec4 v = { a.x*s, a.y*s, a.z*s, a.w*s };
    return v;
}

#define INV_255 0.0039215686
JC_Vec4 jc_colorf(JC_Color c)
{
    JC_Vec4 v = {
        c.r * INV_255,
        c.g * INV_255,
        c.b * INV_255,
        c.a * INV_255,
    };
    return v;
}

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
        a.x + t*(b.x - a.x),
        a.y + t*(b.y - a.y),
        a.z + t*(b.z - a.z),
        a.w + t*(b.w - a.w),
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
        a.x + t*(b.x - a.x),
        a.y + t*(b.y - a.y),
        a.z + t*(b.z - a.z),
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

JC_Matrix jc_matrix_transpose(JC_Matrix a)
{
    JC_Matrix m = a;
    m.m12 = a.m21;
    m.m13 = a.m31;
    m.m14 = a.m41;

    m.m21 = a.m12;
    m.m23 = a.m32;
    m.m24 = a.m42;

    m.m31 = a.m13;
    m.m32 = a.m23;
    m.m34 = a.m43;

    m.m41 = a.m14;
    m.m42 = a.m24;
    m.m43 = a.m34;
    return m;
}

JC_Matrix jc_matrix_inv(JC_Matrix a)
{
    JC_Matrix result;

    float a00 = a.m11, a01 = a.m12, a02 = a.m13, a03 = a.m14;
    float a10 = a.m21, a11 = a.m22, a12 = a.m23, a13 = a.m24;
    float a20 = a.m31, a21 = a.m32, a22 = a.m33, a23 = a.m34;
    float a30 = a.m41, a31 = a.m42, a32 = a.m43, a33 = a.m44;

    float b00 = a00*a11 - a01*a10;
    float b01 = a00*a12 - a02*a10;
    float b02 = a00*a13 - a03*a10;
    float b03 = a01*a12 - a02*a11;
    float b04 = a01*a13 - a03*a11;
    float b05 = a02*a13 - a03*a12;
    float b06 = a20*a31 - a21*a30;
    float b07 = a20*a32 - a22*a30;
    float b08 = a20*a33 - a23*a30;
    float b09 = a21*a32 - a22*a31;
    float b10 = a21*a33 - a23*a31;
    float b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    float inv_det = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    result.m11 = (a11*b11 - a12*b10 + a13*b09)*inv_det;
    result.m12 = (-a01*b11 + a02*b10 - a03*b09)*inv_det;
    result.m13 = (a31*b05 - a32*b04 + a33*b03)*inv_det;
    result.m14 = (-a21*b05 + a22*b04 - a23*b03)*inv_det;
    result.m21 = (-a10*b11 + a12*b08 - a13*b07)*inv_det;
    result.m22 = (a00*b11 - a02*b08 + a03*b07)*inv_det;
    result.m23 = (-a30*b05 + a32*b02 - a33*b01)*inv_det;
    result.m24 = (a20*b05 - a22*b02 + a23*b01)*inv_det;
    result.m31 = (a10*b10 - a11*b08 + a13*b06)*inv_det;
    result.m32 = (-a00*b10 + a01*b08 - a03*b06)*inv_det;
    result.m33 = (a30*b04 - a31*b02 + a33*b00)*inv_det;
    result.m34 = (-a20*b04 + a21*b02 - a23*b00)*inv_det;
    result.m41 = (-a10*b09 + a11*b07 - a12*b06)*inv_det;
    result.m42 = (a00*b09 - a01*b07 + a02*b06)*inv_det;
    result.m43 = (-a30*b03 + a31*b01 - a32*b00)*inv_det;
    result.m44 = (a20*b03 - a21*b01 + a22*b00)*inv_det;

    return result;
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
#define Vec2 JC_Vec2
#define Vec3 JC_Vec3
#define Vec4 JC_Vec4

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
#define draw_rect jc_draw_rect
#define draw_line jc_draw_line
#define draw_triangle jc_draw_triangle

#define begin_mode_3d jc_begin_mode_3d
#define end_mode_3d jc_end_mode_3d
#define disable_depth jc_disable_depth
#define enable_depth jc_enable_depth
#define disable_backface_culling jc_disable_backface_culling
#define enable_backface_culling jc_enable_backface_culling
#define set_shader jc_set_shader
#define unset_shader jc_unset_shader
#define set_shader_uniforms jc_set_shader_uniforms
#define image_sample jc_image_sample

#define rasterize_triangle jc_rasterize_triangle
#define render_geometry jc_render_geometry
#define render_geometry_lines jc_render_geometry_lines
#define draw_model jc_draw_model
#define draw_model_wires jc_draw_model_wires
#define draw_cube jc_draw_cube
#define draw_sphere jc_draw_sphere

#define barycentric_interpolate jc_barycentric_interpolate
#define color_from_int jc_color_from_int
#define colorb jc_colorb
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

#define vec4_add jc_vec4_add
#define vec4_sub jc_vec4_sub
#define vec4_mul jc_vec4_mul
#define vec4_scale jc_vec4_scale
#define colorf jc_colorf
#define vec4_lerp jc_vec4_lerp
#define vec4_transform jc_vec4_transform

#define vec3_add jc_vec3_add
#define vec3_sub jc_vec3_sub
#define vec3_scale jc_vec3_scale
#define vec3_transform jc_vec3_transform
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
#define matrix_transpose jc_matrix_transpose
#define matrix_inv jc_matrix_inv
#endif // JC_PREFIX

#endif // JCANVAS_H
