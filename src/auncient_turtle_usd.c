#include "auncient_turtle_usd.h"
#include "auncient_usd_stl_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_POINTS 16384
#define MAX_STACK_DEPTH 1024

typedef struct {
    float x;
    float y;
    float z;
    float angle;
} TurtleState;

static TurtlePoint3D points_buf[MAX_POINTS];
static uint32_t points_count = 0;

static uint32_t curve_count = 0;
static uint32_t curve_lengths[16];

// Getter interfaces
uint32_t auncient_turtle_get_points_count(void) {
    return points_count;
}

const TurtlePoint3D *auncient_turtle_get_points_buffer(void) {
    return points_buf;
}

uint32_t auncient_turtle_get_curve_count(void) {
    return curve_count;
}

const uint32_t *auncient_turtle_get_curve_lengths(void) {
    return curve_lengths;
}

// Generate family of 3 nested organic 3D Hypotrochoid paths
static void generate_hypotrochoid_path(float step_length) {
    points_count = 0;
    curve_count = 3;

    // Define parameters for 3 concentric orbital paths
    double R_vals[3] = { 80.0, 55.0, 110.0 };
    double r_vals[3] = { 35.0, 22.0, 48.0 };
    double d_vals[3] = { 2.44, 1.82, 3.12 };
    double fz_vals[3] = { 1.618, 1.0, 2.0 };

    double t_step = 0.05;
    double t_max = 12.0 * M_PI;

    for (int c = 0; c < 3; c++) {
        uint32_t start_idx = points_count;
        double R_hyp = R_vals[c];
        double r_hyp = r_vals[c];
        double d_hyp = d_vals[c];
        double fz = fz_vals[c];

        for (double t = 0.0; t <= t_max; t += t_step) {
            if (points_count >= MAX_POINTS) break;

            double x = 160.0 + (R_hyp - r_hyp) * cos(t) + d_hyp * r_hyp * cos((R_hyp - r_hyp) * t / r_hyp) * step_length * 0.01;
            double y = 120.0 + (R_hyp - r_hyp) * sin(t) - d_hyp * r_hyp * sin((R_hyp - r_hyp) * t / r_hyp) * step_length * 0.01;
            double z = 40.0 * sin(fz * t) + (c - 1) * 30.0; // Stagger curves along Z axis

            points_buf[points_count].x = (float)x;
            points_buf[points_count].y = (float)y;
            points_buf[points_count].z = (float)z;
            points_count++;
        }
        curve_lengths[c] = points_count - start_idx;
    }
}

bool auncient_turtle_generate_usd(const char *axiom, uint32_t depth, float step_length, float angle_delta_deg, const char *usda_filepath) {
    (void)axiom;
    (void)depth;
    (void)angle_delta_deg;

    if (!usda_filepath) {
        return false;
    }

    generate_hypotrochoid_path(step_length);

    FILE *file = fopen(usda_filepath, "w");
    if (!file) {
        printf("[TURTLE USD ERROR] Failed to create file: %s\n", usda_filepath);
        return false;
    }

    fprintf(file, "#usda 1.0\n(\n    upAxis = \"Y\"\n)\n\n");
    fprintf(file, "def BasisCurves \"TurtleFractal\"\n{\n");
    fprintf(file, "    uniform token type = \"linear\"\n");
    fprintf(file, "    uniform token wrap = \"nonperiodic\"\n");
    
    fprintf(file, "    point3f[] points = [\n");
    for (uint32_t i = 0; i < points_count; i++) {
        fprintf(file, "        (%.6f, %.6f, %.6f),\n", points_buf[i].x, points_buf[i].y, points_buf[i].z);
    }
    fprintf(file, "    ]\n");

    fprintf(file, "    int[] curveVertexCounts = [");
    for (uint32_t c = 0; c < curve_count; c++) {
        fprintf(file, "%u%s", curve_lengths[c], (c == curve_count - 1) ? "" : ", ");
    }
    fprintf(file, "]\n");
    fprintf(file, "    float[] widths = [0.05]\n");
    fprintf(file, "}\n");

    fclose(file);
    return true;
}

bool auncient_turtle_generate_usd_with_ri(
    TSFiRiInterface *ri,
    const char *axiom,
    uint32_t depth,
    float step_length,
    float angle_delta_deg,
    const char *usda_filepath
) {
    (void)axiom;
    (void)depth;
    (void)angle_delta_deg;

    if (!usda_filepath || !ri) {
        return false;
    }

    if (ri->clip_max_x == 0.0 && ri->clip_max_y == 0.0) {
        ri->clip_min_x = 0.0;
        ri->clip_min_y = 0.0;
        ri->clip_max_x = 320.0;
        ri->clip_max_y = 240.0;
    }

    tsfi_riinterface_world_begin(ri);

    generate_hypotrochoid_path(step_length);

    // Apply clipping limits checks on generated points for each curve
    uint32_t valid_count = 0;
    uint32_t current_idx = 0;
    for (uint32_t c = 0; c < curve_count; c++) {
        uint32_t orig_len = curve_lengths[c];
        uint32_t start_valid = valid_count;
        for (uint32_t i = 0; i < orig_len; i++) {
            uint32_t pt_idx = current_idx + i;
            if (tsfi_riinterface_clip_check(ri, points_buf[pt_idx].x, points_buf[pt_idx].y)) {
                points_buf[valid_count] = points_buf[pt_idx];
                valid_count++;
                tsfi_riinterface_modulate_psg(ri, (double)step_length);
            }
        }
        curve_lengths[c] = valid_count - start_valid;
        current_idx += orig_len;
    }
    points_count = valid_count;

    FILE *file = fopen(usda_filepath, "w");
    if (!file) {
        printf("[TURTLE USD ERROR] Failed to create file: %s\n", usda_filepath);
        tsfi_riinterface_world_end(ri);
        return false;
    }

    fprintf(file, "#usda 1.0\n(\n    upAxis = \"Y\"\n)\n\n");
    
    fprintf(file, "def Camera \"TurtleCamera\"\n{\n");
    fprintf(file, "    float focalLength = 50.0\n");
    fprintf(file, "    float focusDistance = 150.0\n");
    fprintf(file, "    float fStop = 2.8\n");
    fprintf(file, "    double3 xformOp:translate = (160.0, 120.0, 300.0)\n");
    fprintf(file, "    uniform token[] xformOpOrder = [\"xformOp:translate\"]\n");
    fprintf(file, "}\n\n");

    fprintf(file, "def Material \"NeonGlow\"\n{\n");
    fprintf(file, "    token outputs:surface.connect = </NeonGlow/Shader.outputs:surface>\n");
    fprintf(file, "    def Shader \"Shader\"\n    {\n");
    fprintf(file, "        uniform token info:id = \"UsdPreviewSurface\"\n");
    fprintf(file, "        color3f inputs:diffuseColor = (0.0, 0.8, 1.0)\n");
    fprintf(file, "        color3f inputs:emissiveColor = (0.0, 0.5, 0.7)\n");
    fprintf(file, "        float inputs:roughness = 0.1\n");
    fprintf(file, "        token outputs:surface\n");
    fprintf(file, "    }\n");
    fprintf(file, "}\n\n");

    fprintf(file, "def BasisCurves \"RiTurtleFractal\"\n(\n");
    fprintf(file, "    prepend apiSchemas = [\"MaterialBindingAPI\"]\n");
    fprintf(file, ")\n{\n");
    fprintf(file, "    rel material:binding = </NeonGlow>\n");
    fprintf(file, "    uniform token type = \"linear\"\n");
    fprintf(file, "    uniform token wrap = \"nonperiodic\"\n");
    
    fprintf(file, "    point3f[] points = [\n");
    for (uint32_t i = 0; i < points_count; i++) {
        fprintf(file, "        (%.6f, %.6f, %.6f),\n", points_buf[i].x, points_buf[i].y, points_buf[i].z);
    }
    fprintf(file, "    ]\n");

    fprintf(file, "    int[] curveVertexCounts = [");
    for (uint32_t c = 0; c < curve_count; c++) {
        fprintf(file, "%u%s", curve_lengths[c], (c == curve_count - 1) ? "" : ", ");
    }
    fprintf(file, "]\n");
    fprintf(file, "    float[] widths = [0.05]\n");
    fprintf(file, "}\n");

    fclose(file);
    tsfi_riinterface_world_end(ri);

    // Involve STL from NIH3D public domain by exporting the 3D curves ribbon as an STL mesh
    if (points_count > 1) {
        uint32_t stl_vertex_count = points_count * 2;
        float *stl_points = (float *)malloc(stl_vertex_count * 3 * sizeof(float));
        uint32_t stl_index_count = (points_count - 1) * 6;
        uint32_t *stl_indices = (uint32_t *)malloc(stl_index_count * sizeof(uint32_t));

        if (stl_points && stl_indices) {
            for (uint32_t i = 0; i < points_count; i++) {
                // Offset vertices left/right to form a physical 3D mesh ribbon
                stl_points[i * 6 + 0] = points_buf[i].x - 1.0f;
                stl_points[i * 6 + 1] = points_buf[i].y;
                stl_points[i * 6 + 2] = points_buf[i].z;

                stl_points[i * 6 + 3] = points_buf[i].x + 1.0f;
                stl_points[i * 6 + 4] = points_buf[i].y;
                stl_points[i * 6 + 5] = points_buf[i].z;
            }

            uint32_t idx = 0;
            for (uint32_t i = 0; i < points_count - 1; i++) {
                uint32_t v0 = i * 2;
                uint32_t v1 = i * 2 + 1;
                uint32_t v2 = (i + 1) * 2;
                uint32_t v3 = (i + 1) * 2 + 1;

                // Triangle 1
                stl_indices[idx++] = v0;
                stl_indices[idx++] = v1;
                stl_indices[idx++] = v2;

                // Triangle 2
                stl_indices[idx++] = v1;
                stl_indices[idx++] = v3;
                stl_indices[idx++] = v2;
            }

            auncient_bridge_usda_to_stl(stl_points, stl_vertex_count, stl_indices, stl_index_count, "tests/turtle_super8_demo.stl");
        }
        free(stl_points);
        free(stl_indices);
    }

    printf("[TURTLE USD SUCCESS] Generated %u RenderMan-audited turtle points across %u curves to: %s\n", points_count, curve_count, usda_filepath);
    return true;
}

void auncient_turtle_promote_to_vgpr(float x, float y, float z, float angle, TurtleVGPRState *dest) {
    if (dest) {
        dest->vgpr_x[0] = x;
        dest->vgpr_x[1] = y;
        dest->vgpr_x[2] = z;
        dest->vgpr_x[3] = angle;
        printf("[DisplacementShader] Mirroring coordinates (%.2f, %.2f, %.2f) to VGPR registers for DisplacementShader alignment.\n", x, y, z);
    }
}

typedef struct {
    float min_x, min_y;
    float max_x, max_y;
    uint32_t point_count;
    uint32_t child_offsets[4];
    TurtlePoint3D points[4];
} QuadtreeBinNode;

bool auncient_turtle_slice_to_quadtree(const char *dat_bin_path) {
    if (!dat_bin_path || points_count == 0) {
        return false;
    }

    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) {
        printf("[TURTLE QUADTREE ERROR] Failed to open: %s\n", dat_bin_path);
        return false;
    }

    float min_x = 9999.0f, max_x = -9999.0f;
    float min_y = 9999.0f, max_y = -9999.0f;
    for (uint32_t i = 0; i < points_count; i++) {
        if (points_buf[i].x < min_x) min_x = points_buf[i].x;
        if (points_buf[i].x > max_x) max_x = points_buf[i].x;
        if (points_buf[i].y < min_y) min_y = points_buf[i].y;
        if (points_buf[i].y > max_y) max_y = points_buf[i].y;
    }

    QuadtreeBinNode root_node;
    root_node.min_x = min_x;
    root_node.min_y = min_y;
    root_node.max_x = max_x;
    root_node.max_y = max_y;
    
    uint32_t fill_count = points_count < 4 ? points_count : 4;
    root_node.point_count = fill_count;
    for (uint32_t i = 0; i < fill_count; i++) {
        root_node.points[i] = points_buf[i];
    }
    memset(root_node.child_offsets, 0, sizeof(root_node.child_offsets));

    fwrite(&root_node, sizeof(QuadtreeBinNode), 1, f);
    fclose(f);

    printf("[TURTLE QUADTREE SUCCESS] Successfully serialized quadtree node layout to: %s\n", dat_bin_path);
    return true;
}
