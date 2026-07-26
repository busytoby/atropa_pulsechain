#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_texgen.h"

#define LANES 4
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define WARNING_DRONE  110
#define GRID_SIZE      8

typedef struct {
    uint32_t max_allowed_transactions; // Transaction count ceiling
    uint64_t consensus_threshold;      // Accumulator quorum threshold
    uint32_t baseline_color;           // Black SGPR base
} sgpr_bank_t;

typedef struct {
    uint32_t account_id[LANES];
    int64_t balance[LANES];
    uint64_t accumulated_votes[LANES]; // Accumulator values
    uint32_t status[LANES];            // Red VGPR alert indicator
} vgpr_bank_t;

typedef struct {
    uint32_t border_color;
    uint32_t psg_frequency;
    bool blame_quarantine;
    uint32_t transaction_count;
    // Conspiracy-inspired procedural properties
    double lissajous_x;
    double lissajous_y;
    double lissajous_z;
    color_pixel_t texture_grid[GRID_SIZE][GRID_SIZE];
} huc_ocean_system_t;

// Wrapper calling the standardized texture generator and converting output to console grid format
static void synthesize_texture_grid_rgb(huc_ocean_system_t *huc, double phase, int mode, const tsfi_texgen_params_t *params) {
    uint8_t output_rgba[GRID_SIZE * GRID_SIZE * 4];
    tsfi_texgen_render(output_rgba, GRID_SIZE, GRID_SIZE, phase, mode, params);
    
    const char glyphs[] = " .:-=+*#%@";
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int out_idx = (y * GRID_SIZE + x) * 4;
            huc->texture_grid[y][x].r = output_rgba[out_idx];
            huc->texture_grid[y][x].g = output_rgba[out_idx+1];
            huc->texture_grid[y][x].b = output_rgba[out_idx+2];
            
            double h_center = (output_rgba[out_idx] + output_rgba[out_idx+1] + output_rgba[out_idx+2]) / 3.0;
            int idx = (int)(h_center / 25.6);
            if (idx < 0) idx = 0;
            if (idx > 9) idx = 9;
            huc->texture_grid[y][x].glyph = glyphs[idx];
        }
    }
}

typedef struct {
    float x;
    float y;
    float z;
} vertex3d_t;

typedef struct {
    float min_x;
    float min_y;
    float max_x;
    float max_y;
    int32_t child_pointers[4];
    int32_t is_leaf;
    int32_t vertex_count;
    vertex3d_t vertices[4];
} quadtree_node_t;

static uint32_t calculate_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

static void serialize_quadtree_to_bin(const char *filename, const quadtree_node_t *node) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    fwrite(node, sizeof(quadtree_node_t), 1, f);
    
    // Calculate and append CRC32 checksum trailer
    uint32_t crc = calculate_crc32((const uint8_t*)node, sizeof(quadtree_node_t));
    fwrite(&crc, sizeof(uint32_t), 1, f);
    fclose(f);
}

// Decodes a .dna byte stream representing operator sequences (1: Taper, 2: Twist, 3: Displace)
// Uses compressed 16-bit fixed point parameters (divided by 10000.0f)
static void decode_dna_and_apply(vertex3d_t *vertices, int count, const uint8_t *dna, int dna_len) {
    int pos = 0;
    while (pos < dna_len) {
        if (pos >= dna_len) break;
        uint8_t op_id = dna[pos++];
        
        if (pos + 2 > dna_len) break;
        int16_t fixed_val;
        memcpy(&fixed_val, &dna[pos], 2);
        pos += 2;
        
        float val = (float)fixed_val / 10000.0f;
        
        if (op_id == 1) {
            // Taper Operator decoded from DNA
            for (int v = 0; v < count; v++) {
                float scale = 1.0f - val * (vertices[v].z + 1.0f) * 0.5f;
                vertices[v].x *= scale;
                vertices[v].y *= scale;
            }
        } else if (op_id == 2) {
            // Twist Operator decoded from DNA
            for (int v = 0; v < count; v++) {
                float angle = vertices[v].z * val;
                float tx = vertices[v].x * tsfi_texgen_cos(angle) - vertices[v].y * tsfi_texgen_sin(angle);
                float ty = vertices[v].x * tsfi_texgen_sin(angle) + vertices[v].y * tsfi_texgen_cos(angle);
                vertices[v].x = tx;
                vertices[v].y = ty;
            }
        } else if (op_id == 3) {
            // Displace Operator decoded from DNA
            for (int v = 0; v < count; v++) {
                vertices[v].x += val * tsfi_texgen_sin(vertices[v].z * 5.0f);
                vertices[v].y += val * tsfi_texgen_cos(vertices[v].z * 5.0f);
            }
        }
    }
}

// 2. Procedural 3D Mesh / Lissajous Pathing using DNA stream decoding & Quadtree serialization
static void update_lissajous_mesh(huc_ocean_system_t *huc, double signal) {
    vertex3d_t vertices[1] = {
        {
            .x = (float)tsfi_texgen_sin(signal * 2.5),
            .y = (float)tsfi_texgen_cos(signal * 1.8),
            .z = (float)tsfi_texgen_sin(signal * 3.2 + 0.5)
        }
    };
    
    // Serialized DNA instruction stream using 16-bit fixed point mapping (Taper 0.2f, Twist 0.5f, Displace 0.1f)
    int16_t fixed_taper = (int16_t)(0.2f * 10000.0f);
    int16_t fixed_twist = (int16_t)(0.5f * 10000.0f);
    int16_t fixed_displace = (int16_t)(0.1f * 10000.0f);
    
    uint8_t dna[9];
    dna[0] = 1; memcpy(&dna[1], &fixed_taper, 2);
    dna[3] = 2; memcpy(&dna[4], &fixed_twist, 2);
    dna[6] = 3; memcpy(&dna[7], &fixed_displace, 2);
    
    // Decode and apply DNA operators
    decode_dna_and_apply(vertices, 1, dna, 9);
    
    // Save coordinate as quadtree root slice to .dat.bin file
    quadtree_node_t root_node = {
        .min_x = -2.0f, .min_y = -2.0f, .max_x = 2.0f, .max_y = 2.0f,
        .child_pointers = {-1, -1, -1, -1},
        .is_leaf = 1,
        .vertex_count = 1,
        .vertices = { {vertices[0].x, vertices[0].y, vertices[0].z} }
    };
    serialize_quadtree_to_bin("targ_map_quadtree.dat.bin", &root_node);
    
    huc->lissajous_x = vertices[0].x;
    huc->lissajous_y = vertices[0].y;
    huc->lissajous_z = vertices[0].z;
}

// 3. Low-Level Tape Ingest process
static void process_tape_ingest_v6(huc_ocean_system_t *huc,
                                   const sgpr_bank_t *sgprs,
                                   vgpr_bank_t *vgprs) {
    printf("[PROCESS] Executing Level 6 Hogan-Hudson Ocean Tape Ingest...\n");
    fflush(stdout);

    // Initialized parameter struct under 200 bytes limit
    tsfi_texgen_params_t params = {
        .seed = 999U,
        .phase_scale = 1.5f,
        .twirl_strength = 2.0f,
        .blend_mode = 1U,
        .light_x = 0.57735f,
        .light_y = 0.57735f,
        .light_z = 0.57735f,
        .persistence = 0.5f,
        .lacunarity = 2.0f,
        .octaves = 3U,
        .brightness = 0.0f,
        .contrast = 1.0f,
        .wave_type = 0U,
        .feedback_strength = 0.5f,
        .clut_preset = 0U,
        .vignette_strength = 0.3f,
        .threshold_limit = 0.0f,
        .shell_ao = 0.4f,
        .anisotropy = 0.5f
    };

    for (int lane = 0; lane < LANES; lane++) {
        uint32_t acc = vgprs->account_id[lane];
        uint64_t votes = vgprs->accumulated_votes[lane];
        
        huc->border_color = sgprs->baseline_color;

        // Check Transaction Count Limit
        if (huc->transaction_count >= sgprs->max_allowed_transactions) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED;
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [INGEST FAIL] Lane %d: Transaction count ceiling exceeded! Ingest aborted.\n", lane);
            fflush(stdout);
            break;
        }

        // Check Accumulator Quorum Threshold
        if (votes < sgprs->consensus_threshold) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED;
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [INGEST FAIL] Lane %d: Account %d lacks sufficient quorum votes (%lu < %lu)! Ingest aborted.\n", 
                   lane, acc, votes, sgprs->consensus_threshold);
            fflush(stdout);
            break;
        }

        huc->transaction_count++;
        vgprs->status[lane] = COLOR_CYAN;
        huc->border_color = COLOR_CYAN;

        double lfo_mod = tsfi_texgen_sin(huc->transaction_count * 0.8) * 50.0;
        huc->psg_frequency = (uint32_t)(261.0 + (huc->transaction_count * 20.0) + lfo_mod);

        // Dynamically alternate blend modes between Multiply (1) and Difference (2)
        params.blend_mode = (lane % 2 == 0) ? 1U : 2U;

        // Update procedural visualizers
        synthesize_texture_grid_rgb(huc, huc->transaction_count * params.phase_scale, lane % 4, &params);
        update_lissajous_mesh(huc, huc->transaction_count * 1.2);

        printf("   [INGEST PASS] Lane %d: Account %d balance reconciled. Color: 0x%06X. LFO Freq: %u Hz.\n", 
               lane, acc, huc->border_color, huc->psg_frequency);
        
        // Output procedural text matrix
        printf("   [PROCEDURAL TEXTURE LAYER] (%s Mode):\n", (lane % 2 == 0) ? "Normal Map" : "Gradient Map");
        for (int y = 0; y < GRID_SIZE; y++) {
            printf("      ");
            for (int x = 0; x < GRID_SIZE; x++) {
                color_pixel_t px = huc->texture_grid[y][x];
                printf("\033[38;2;%u;%u;%um%c", px.r, px.g, px.b, px.glyph);
            }
            printf("\033[0m\n");
        }
        printf("   [LORE GEOMETRY MESH] Coordinate: (%.3f, %.3f, %.3f)\n", 
               huc->lissajous_x, huc->lissajous_y, huc->lissajous_z);
        fflush(stdout);
    }
}

typedef struct {
    float m[3][3];
    float t[3];
} bone_transform_t;

static float interpolate_hermite(float p0, float m0, float p1, float m1, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return (2.0f*t3 - 3.0f*t2 + 1.0f)*p0 + (t3 - 2.0f*t2 + t)*m0 + (-2.0f*t3 + 3.0f*t2)*p1 + (t3 - t2)*m1;
}

static void apply_vertex_skinning(const vertex3d_t *src, vertex3d_t *dest, const bone_transform_t *bones, const float *weights, const int32_t *bone_indices, int weight_count) {
    dest->x = 0.0f;
    dest->y = 0.0f;
    dest->z = 0.0f;
    for (int i = 0; i < weight_count; i++) {
        int b_idx = bone_indices[i];
        const bone_transform_t *b = &bones[b_idx];
        float w = weights[i];
        
        float rx = b->m[0][0]*src->x + b->m[0][1]*src->y + b->m[0][2]*src->z + b->t[0];
        float ry = b->m[1][0]*src->x + b->m[1][1]*src->y + b->m[1][2]*src->z + b->t[1];
        float rz = b->m[2][0]*src->x + b->m[2][1]*src->y + b->m[2][2]*src->z + b->t[2];
        
        dest->x += rx * w;
        dest->y += ry * w;
        dest->z += rz * w;
    }
}

typedef struct {
    float x, y, z;
    float vx, vy, vz;
} particle3d_t;

static void update_particles(particle3d_t *particles, int count, float gravity, float wind_x, float wind_y) {
    for (int i = 0; i < count; i++) {
        particles[i].vx += wind_x;
        particles[i].vy += wind_y;
        particles[i].vz -= gravity;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].z += particles[i].vz;
    }
}

static float sdf_sphere(const vertex3d_t *p, float radius) {
    return sqrt(p->x*p->x + p->y*p->y + p->z*p->z) - radius;
}

static float sdf_union(float d1, float d2) {
    return d1 < d2 ? d1 : d2;
}

static void get_torus_vertex(vertex3d_t *vertex, float u, float v, float r_major, float r_minor) {
    float theta = u * 2.0f * 3.14159265f;
    float phi = v * 2.0f * 3.14159265f;
    float cos_theta = tsfi_texgen_cos(theta);
    float sin_theta = tsfi_texgen_sin(theta);
    float cos_phi = tsfi_texgen_cos(phi);
    float sin_phi = tsfi_texgen_sin(phi);
    
    vertex->x = (r_major + r_minor * cos_phi) * cos_theta;
    vertex->y = (r_major + r_minor * cos_phi) * sin_theta;
    vertex->z = r_minor * sin_phi;
}

static void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b) {
    float c = v * s;
    float x = c * (1.0f - fabs(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r1 = 0.0f, g1 = 0.0f, b1 = 0.0f;
    if (h >= 0.0f && h < 60.0f) { r1 = c; g1 = x; }
    else if (h >= 60.0f && h < 120.0f) { r1 = x; g1 = c; }
    else if (h >= 120.0f && h < 180.0f) { g1 = c; b1 = x; }
    else if (h >= 180.0f && h < 240.0f) { g1 = x; b1 = c; }
    else if (h >= 240.0f && h < 300.0f) { r1 = x; b1 = c; }
    else { r1 = c; b1 = x; }
    *r = (uint8_t)((r1 + m) * 255.0f);
    *g = (uint8_t)((g1 + m) * 255.0f);
    *b = (uint8_t)((b1 + m) * 255.0f);
}

static void get_cylinder_vertex(vertex3d_t *vertex, float u, float h, float radius) {
    float theta = u * 2.0f * 3.14159265f;
    vertex->x = radius * tsfi_texgen_cos(theta);
    vertex->y = radius * tsfi_texgen_sin(theta);
    vertex->z = h;
}

typedef struct {
    float r[3]; // Right
    float u[3]; // Up
    float f[3]; // Forward
} camera_frame_t;

static void compute_lookat_frame(camera_frame_t *frame, const vertex3d_t *eye, const vertex3d_t *target) {
    float fx = target->x - eye->x;
    float fy = target->y - eye->y;
    float fz = target->z - eye->z;
    float flen = sqrt(fx*fx + fy*fy + fz*fz);
    if (flen > 0.0f) { fx /= flen; fy /= flen; fz /= flen; }
    frame->f[0] = fx; frame->f[1] = fy; frame->f[2] = fz;
    
    float ux = 0.0f, uy = 0.0f, uz = 1.0f;
    
    float rx = fy*uz - fz*uy;
    float ry = fz*ux - fx*uz;
    float rz = fx*uy - fy*ux;
    float rlen = sqrt(rx*rx + ry*ry + rz*rz);
    if (rlen > 0.0f) { rx /= rlen; ry /= rlen; rz /= rlen; }
    frame->r[0] = rx; frame->r[1] = ry; frame->r[2] = rz;
    
    frame->u[0] = ry*fz - rz*fy;
    frame->u[1] = rz*fx - rx*fz;
    frame->u[2] = rx*fy - ry*fx;
}

static float interpolate_catmull_rom(float p0, float p1, float p2, float p3, float t) {
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
}

static void displace_vertex_by_texture(vertex3d_t *vertex, const uint8_t *texture_rgba, int width, int height, float u, float v, float amount) {
    int x = (int)(u * (width - 1)) % width;
    int y = (int)(v * (height - 1)) % height;
    if (x < 0) x += width;
    if (y < 0) y += height;
    int idx = (y * width + x) * 4;
    float height_val = (float)texture_rgba[idx] / 255.0f; // Red channel as height
    vertex->z += height_val * amount;
}

static void run_digital_dynamite_tests(void) {
    printf("[TEST] Running Digital Dynamite hermite interpolation and texture displacement tests...\n");
    fflush(stdout);
    
    // 1. Test Catmull-Rom Keyframe Interpolation
    float p0 = 0.0f, p1 = 1.0f, p2 = 2.0f, p3 = 3.0f;
    float val = interpolate_catmull_rom(p0, p1, p2, p3, 0.5f);
    assert(val >= 0.9f && val <= 2.1f);
    printf("   ✓ Catmull-Rom interpolation verified successfully.\n");
    
    // 2. Test Texture-Driven Vertex Displacement
    uint8_t mock_tex[16] = {
        255, 0, 0, 255,   0, 0, 0, 255,
        0, 0, 0, 255,     128, 0, 0, 255
    };
    vertex3d_t vertex = {0.0f, 0.0f, 0.0f};
    displace_vertex_by_texture(&vertex, mock_tex, 2, 2, 0.0f, 0.0f, 1.5f);
    assert(vertex.z > 1.4f);
    printf("   ✓ Texture-driven vertex displacement verified successfully.\n");
    
    // 3. Test Hermite Curve Spline Pathing
    float h_val = interpolate_hermite(0.0f, 1.0f, 1.0f, 1.0f, 0.5f);
    assert(h_val >= 0.4f && h_val <= 0.6f);
    printf("   ✓ Hermite spline pathing verified successfully.\n");
    
    // 4. Test Skeletal Bone Transform & Weight-Based Skinning
    bone_transform_t bones[1] = {
        {
            .m = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
            .t = { 1.0f, 2.0f, 3.0f }
        }
    };
    vertex3d_t src_v = { 0.0f, 0.0f, 0.0f };
    vertex3d_t dest_v;
    float weights[1] = { 1.0f };
    int32_t bone_indices[1] = { 0 };
    apply_vertex_skinning(&src_v, &dest_v, bones, weights, bone_indices, 1);
    assert(dest_v.x > 0.9f && dest_v.y > 1.9f && dest_v.z > 2.9f);
    printf("   ✓ Skeletal weight-based skinning verified successfully.\n");
    
    // 5. Test Procedural Particle Flow Field
    particle3d_t p = { 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f };
    update_particles(&p, 1, 0.5f, 0.1f, -0.1f);
    assert(p.z < 10.0f && p.vx > 0.0f);
    printf("   ✓ Procedural particle flow field verified successfully.\n");
    
    // 6. Test SDF Sphere and Union Operator
    vertex3d_t sp_v = { 0.0f, 0.0f, 0.5f };
    float d_sphere = sdf_sphere(&sp_v, 1.0f);
    float d_union = sdf_union(d_sphere, 5.0f);
    assert(d_sphere < 0.0f && d_union < 0.0f);
    printf("   ✓ SDF primitives and Union operations verified successfully.\n");
    
    // 7. Test Procedural Torus Generator
    vertex3d_t tor_v;
    get_torus_vertex(&tor_v, 0.25f, 0.25f, 2.0f, 0.5f);
    assert(tor_v.x <= 2.1f && tor_v.z <= 0.6f);
    printf("   ✓ Torus vertex generator verified successfully.\n");
    
    // 8. Test LookAt View Matrix Generator
    vertex3d_t eye = {0.0f, -5.0f, 0.0f};
    vertex3d_t target = {0.0f, 0.0f, 0.0f};
    camera_frame_t frame;
    compute_lookat_frame(&frame, &eye, &target);
    assert(frame.f[1] > 0.9f);
    printf("   ✓ LookAt view frame computation verified successfully.\n");
    
    // 9. Test HSV to RGB Conversion
    uint8_t cr = 0, cg = 0, cb = 0;
    hsv_to_rgb(180.0f, 1.0f, 1.0f, &cr, &cg, &cb);
    assert(cr == 0 && cg == 255 && cb == 255);
    printf("   ✓ HSV to RGB conversion verified successfully.\n");
    
    // 10. Test Procedural Cylinder Generator
    vertex3d_t cyl_v;
    get_cylinder_vertex(&cyl_v, 0.25f, 1.5f, 2.0f);
    assert(cyl_v.x <= 0.1f && cyl_v.y > 1.9f && cyl_v.z > 1.4f);
    printf("   ✓ Cylinder vertex generator verified successfully.\n");
    fflush(stdout);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN-HUDSON OCEAN INTEGRATION LEVEL 6 (DEMOSCENE EDITION)\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialise standardized texgen tables
    tsfi_texgen_init();
    
    // Execute Digital Dynamite unit tests
    run_digital_dynamite_tests();

    // SGPR settings
    sgpr_bank_t sgprs = {
        .max_allowed_transactions = 2,
        .consensus_threshold = 600,
        .baseline_color = COLOR_BLACK
    };

    // VGPR settings
    vgpr_bank_t vgprs = {
        .account_id = { 101, 102, 103, 104 },
        .balance = { 4000, 2500, 3000, 5000 },
        .accumulated_votes = {
            600, // Lane 0: Pass
            600, // Lane 1: Pass
            300, // Lane 2: Fail
            600  // Lane 3: Fail
        },
        .status = { 0 }
    };

    huc_ocean_system_t huc = {
        .border_color = COLOR_BLACK,
        .psg_frequency = 261,
        .blame_quarantine = false,
        .transaction_count = 0,
        .lissajous_x = 0.0,
        .lissajous_y = 0.0,
        .lissajous_z = 0.0
    };

    process_tape_ingest_v6(&huc, &sgprs, &vgprs);

    // Verify assertions
    printf("[TEST] Verifying combined system states...\n");
    fflush(stdout);

    assert(vgprs.status[0] == COLOR_CYAN);
    assert(vgprs.status[1] == COLOR_CYAN);
    assert(vgprs.status[2] == COLOR_RED);
    assert(huc.border_color == COLOR_RED);
    assert(huc.psg_frequency == WARNING_DRONE);
    assert(huc.blame_quarantine == true);
    
    printf("   ✓ System registers, dynamic loader signals, and LFOs verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HOGAN-HUDSON LEVEL 6 INTEGRATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
