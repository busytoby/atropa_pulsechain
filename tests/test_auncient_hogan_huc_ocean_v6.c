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

// Decodes a .dna byte stream representing operator sequences (1: Taper, 2: Twist, 3: Displace)
static void decode_dna_and_apply(vertex3d_t *vertices, int count, const uint8_t *dna, int dna_len) {
    int pos = 0;
    while (pos < dna_len) {
        if (pos >= dna_len) break;
        uint8_t op_id = dna[pos++];
        
        if (pos + 4 > dna_len) break;
        float val;
        memcpy(&val, &dna[pos], 4);
        pos += 4;
        
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

// 2. Procedural 3D Mesh / Lissajous Pathing using DNA stream decoding
static void update_lissajous_mesh(huc_ocean_system_t *huc, double signal) {
    vertex3d_t vertices[1] = {
        {
            .x = (float)tsfi_texgen_sin(signal * 2.5),
            .y = (float)tsfi_texgen_cos(signal * 1.8),
            .z = (float)tsfi_texgen_sin(signal * 3.2 + 0.5)
        }
    };
    
    // Serialized DNA instruction stream (Taper 0.2f, Twist 0.5f, Displace 0.1f)
    float val_taper = 0.2f;
    float val_twist = 0.5f;
    float val_displace = 0.1f;
    
    uint8_t dna[15];
    dna[0] = 1; memcpy(&dna[1], &val_taper, 4);
    dna[5] = 2; memcpy(&dna[6], &val_twist, 4);
    dna[10] = 3; memcpy(&dna[11], &val_displace, 4);
    
    // Decode and apply DNA operators
    decode_dna_and_apply(vertices, 1, dna, 15);
    
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

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN-HUDSON OCEAN INTEGRATION LEVEL 6 (DEMOSCENE EDITION)\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialise standardized texgen tables
    tsfi_texgen_init();

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
