#ifndef TSFI_RAY_TRACER_H
#define TSFI_RAY_TRACER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    CGM_PRIM_SPHERE,
    CGM_PRIM_BOX,
    CGM_PRIM_PLANE
} tsfi_cgm_prim_type;

typedef struct {
    float x, y, z;
} tsfi_rt_vec3;

typedef struct {
    tsfi_cgm_prim_type type;
    tsfi_rt_vec3 position;
    tsfi_rt_vec3 color;      // Diffuse color (RGB)
    float param1;            // Radius for sphere, size, etc.
    tsfi_rt_vec3 param_vec;  // Extra dimensions
} tsfi_cgm_primitive;

typedef struct {
    tsfi_cgm_primitive primitives[16];
    int primitive_count;
    tsfi_rt_vec3 light_dir;
    tsfi_rt_vec3 ambient_color;
} tsfi_cgm_scene;

void tsfi_cgm_scene_init(tsfi_cgm_scene *scene);
int tsfi_cgm_scene_add_primitive(tsfi_cgm_scene *scene, tsfi_cgm_prim_type type, tsfi_rt_vec3 pos, tsfi_rt_vec3 color, float param1, tsfi_rt_vec3 param_vec);
int tsfi_cgm_parse_command(tsfi_cgm_scene *scene, const uint8_t *cmd_bytes, size_t len);
int tsfi_ray_tracer_render(const tsfi_cgm_scene *scene, uint32_t *image_out, int width, int height);
int tsfi_cad_parse_punched_card(tsfi_cgm_scene *scene, const char *card_80);
int tsfi_gks_to_ray_tracer(const uint8_t *gks_stream, int len, tsfi_cgm_scene *scene);
int tsfi_oscar_trace_to_ray_tracer(const float *coefficients, int order, float x_start, float x_end, int steps, tsfi_cgm_scene *scene);
int tsfi_fips54_com_to_ray_tracer(int rows, int cols, float spacing, tsfi_cgm_scene *scene);

#endif // TSFI_RAY_TRACER_H
