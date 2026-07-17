#include "tsfi_ray_tracer.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

static tsfi_rt_vec3 vec3_sub(tsfi_rt_vec3 a, tsfi_rt_vec3 b) {
    return (tsfi_rt_vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static float vec3_dot(tsfi_rt_vec3 a, tsfi_rt_vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static tsfi_rt_vec3 vec3_normalize(tsfi_rt_vec3 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        return (tsfi_rt_vec3){v.x / len, v.y / len, v.z / len};
    }
    return v;
}

void tsfi_cgm_scene_init(tsfi_cgm_scene *scene) {
    if (!scene) return;
    scene->primitive_count = 0;
    memset(scene->primitives, 0, sizeof(scene->primitives));
    scene->light_dir = vec3_normalize((tsfi_rt_vec3){0.5f, 1.0f, -0.5f});
    scene->ambient_color = (tsfi_rt_vec3){0.1f, 0.1f, 0.1f};
}

int tsfi_cgm_scene_add_primitive(tsfi_cgm_scene *scene, tsfi_cgm_prim_type type, tsfi_rt_vec3 pos, tsfi_rt_vec3 color, float param1, tsfi_rt_vec3 param_vec) {
    if (!scene || scene->primitive_count >= 16) return -1;
    tsfi_cgm_primitive *p = &scene->primitives[scene->primitive_count];
    p->type = type;
    p->position = pos;
    p->color = color;
    p->param1 = param1;
    p->param_vec = param_vec;
    scene->primitive_count++;
    return 0;
}

int tsfi_cgm_parse_command(tsfi_cgm_scene *scene, const uint8_t *cmd_bytes, size_t len) {
    if (!scene || !cmd_bytes || len < 4) return -1;
    uint8_t op = cmd_bytes[0];
    if (op == 0x01 && len >= 28) {
        // Add Sphere: op(1) + pad(3) + pos(12) + rad(4) + color(8)
        tsfi_rt_vec3 pos;
        memcpy(&pos, &cmd_bytes[4], 12);
        float rad;
        memcpy(&rad, &cmd_bytes[16], 4);
        float r = (float)cmd_bytes[20] / 255.0f;
        float g = (float)cmd_bytes[21] / 255.0f;
        float b = (float)cmd_bytes[22] / 255.0f;
        tsfi_rt_vec3 color = {r, g, b};
        return tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_SPHERE, pos, color, rad, (tsfi_rt_vec3){0,0,0});
    } else if (op == 0x02 && len >= 28) {
        // Add Plane: op(1) + pad(3) + normal(12) + d(4) + color(8)
        tsfi_rt_vec3 norm;
        memcpy(&norm, &cmd_bytes[4], 12);
        float d;
        memcpy(&d, &cmd_bytes[16], 4);
        float r = (float)cmd_bytes[20] / 255.0f;
        float g = (float)cmd_bytes[21] / 255.0f;
        float b = (float)cmd_bytes[22] / 255.0f;
        tsfi_rt_vec3 color = {r, g, b};
        return tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_PLANE, norm, color, d, (tsfi_rt_vec3){0,0,0});
    } else if (op == 0x03 && len >= 16) {
        // Set Light: op(1) + pad(3) + dir(12)
        tsfi_rt_vec3 dir;
        memcpy(&dir, &cmd_bytes[4], 12);
        scene->light_dir = vec3_normalize(dir);
        return 0;
    }
    return -2;
}

// Ray-primitive intersection checkers
static int intersect_sphere(tsfi_rt_vec3 orig, tsfi_rt_vec3 dir, tsfi_rt_vec3 center, float radius, float *t_out) {
    tsfi_rt_vec3 oc = vec3_sub(orig, center);
    float a = vec3_dot(dir, dir);
    float b = 2.0f * vec3_dot(oc, dir);
    float c = vec3_dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return 0;
    float t0 = (-b - sqrtf(discriminant)) / (2.0f * a);
    if (t0 > 0.001f) {
        *t_out = t0;
        return 1;
    }
    return 0;
}

static int intersect_plane(tsfi_rt_vec3 orig, tsfi_rt_vec3 dir, tsfi_rt_vec3 normal, float d, float *t_out) {
    float denom = vec3_dot(normal, dir);
    if (fabsf(denom) > 1e-6) {
        float t = (d - vec3_dot(normal, orig)) / denom;
        if (t >= 0.001f) {
            *t_out = t;
            return 1;
        }
    }
    return 0;
}

int tsfi_ray_tracer_render(const tsfi_cgm_scene *scene, uint32_t *image_out, int width, int height) {
    if (!scene || !image_out || width <= 0 || height <= 0) return -1;

    tsfi_rt_vec3 orig = {0.0f, 0.0f, 0.0f};

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Map pixel to normalized device coordinates (-1 to 1)
            float px = (2.0f * ((float)x + 0.5f) / (float)width - 1.0f) * ((float)width / (float)height);
            float py = 1.0f - 2.0f * ((float)y + 0.5f) / (float)height;

            tsfi_rt_vec3 dir = vec3_normalize((tsfi_rt_vec3){px, py, 1.0f});

            float t_min = 1e9f;
            int hit_idx = -1;

            for (int i = 0; i < scene->primitive_count; i++) {
                float t = 0.0f;
                int hit = 0;
                if (scene->primitives[i].type == CGM_PRIM_SPHERE) {
                    hit = intersect_sphere(orig, dir, scene->primitives[i].position, scene->primitives[i].param1, &t);
                } else if (scene->primitives[i].type == CGM_PRIM_PLANE) {
                    hit = intersect_plane(orig, dir, scene->primitives[i].position, scene->primitives[i].param1, &t);
                }

                if (hit && t < t_min) {
                    t_min = t;
                    hit_idx = i;
                }
            }

            if (hit_idx != -1) {
                const tsfi_cgm_primitive *prim = &scene->primitives[hit_idx];
                // Intersection point
                tsfi_rt_vec3 hit_pt = {orig.x + dir.x * t_min, orig.y + dir.y * t_min, orig.z + dir.z * t_min};
                tsfi_rt_vec3 normal = {0,0,0};

                if (prim->type == CGM_PRIM_SPHERE) {
                    normal = vec3_normalize(vec3_sub(hit_pt, prim->position));
                } else if (prim->type == CGM_PRIM_PLANE) {
                    normal = prim->position; // Normal is stored in position
                }

                // Diffuse lighting
                float diffuse = vec3_dot(normal, scene->light_dir);
                if (diffuse < 0.0f) diffuse = 0.0f;

                float r = prim->color.x * (scene->ambient_color.x + diffuse);
                float g = prim->color.y * (scene->ambient_color.y + diffuse);
                float b = prim->color.z * (scene->ambient_color.z + diffuse);

                if (r > 1.0f) r = 1.0f;
                if (g > 1.0f) g = 1.0f;
                if (b > 1.0f) b = 1.0f;

                uint8_t ir = (uint8_t)(r * 255.0f);
                uint8_t ig = (uint8_t)(g * 255.0f);
                uint8_t ib = (uint8_t)(b * 255.0f);

                image_out[y * width + x] = (0xFFULL << 24) | (ir << 16) | (ig << 8) | ib;
            } else {
                // Background color (sky gradient)
                float factor = 0.5f * (dir.y + 1.0f);
                uint8_t r = (uint8_t)((1.0f - factor) * 30.0f + factor * 100.0f);
                uint8_t g = (uint8_t)((1.0f - factor) * 30.0f + factor * 150.0f);
                uint8_t b = (uint8_t)((1.0f - factor) * 40.0f + factor * 220.0f);
                image_out[y * width + x] = (0xFFULL << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

    return 0;
}

int tsfi_cad_parse_punched_card(tsfi_cgm_scene *scene, const char *card_80) {
    if (!scene || !card_80) return -1;

    char card_copy[81];
    strncpy(card_copy, card_80, 80);
    card_copy[80] = '\0';

    if (strncmp(card_copy, "CAD_SPHERE", 10) == 0) {
        float x = 0.0f, y = 0.0f, z = 0.0f, rad = 1.0f;
        char col_char = 'R';
        // Format: CAD_SPHERE X:-0.5 Y:1.0 Z:4.0 R:1.2 COLOR:R
        int parsed = sscanf(card_copy, "CAD_SPHERE X:%f Y:%f Z:%f R:%f COLOR:%c", &x, &y, &z, &rad, &col_char);
        if (parsed < 5) return -2;

        tsfi_rt_vec3 color = {1.0f, 0.0f, 0.0f}; // Default red
        if (col_char == 'G') color = (tsfi_rt_vec3){0.0f, 1.0f, 0.0f};
        else if (col_char == 'B') color = (tsfi_rt_vec3){0.0f, 0.0f, 1.0f};

        return tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_SPHERE, (tsfi_rt_vec3){x, y, z}, color, rad, (tsfi_rt_vec3){0,0,0});
    } else if (strncmp(card_copy, "CAD_LIGHT", 9) == 0) {
        float x = 0.0f, y = 1.0f, z = -0.5f;
        int parsed = sscanf(card_copy, "CAD_LIGHT X:%f Y:%f Z:%f", &x, &y, &z);
        if (parsed < 3) return -2;
        scene->light_dir.x = x;
        scene->light_dir.y = y;
        scene->light_dir.z = z;
        return 0;
    }
    return -2;
}

int tsfi_gks_to_ray_tracer(const uint8_t *gks_stream, int len, tsfi_cgm_scene *scene) {
    if (!gks_stream || !scene) return -1;

    int prim_type = 0;
    int point_count = 0;
    extern int tsfi_fips120_parse_gks_primitive(const uint8_t *stream, int length, int *out_type, int *out_pts);
    int res = tsfi_fips120_parse_gks_primitive(gks_stream, len, &prim_type, &point_count);
    if (res != 0) return res;

    for (int i = 0; i < point_count; i++) {
        float x = (float)gks_stream[2 + 2 * i];
        float y = (float)gks_stream[2 + 2 * i + 1];
        tsfi_rt_vec3 pos = { x / 10.0f - 2.0f, y / 10.0f - 2.0f, 5.0f };
        tsfi_rt_vec3 color = { 0.0f, 0.0f, 1.0f }; // Blue GKS nodes
        tsfi_cgm_scene_add_primitive(scene, CGM_PRIM_SPHERE, pos, color, 0.5f, (tsfi_rt_vec3){0,0,0});
    }

    return 0;
}
