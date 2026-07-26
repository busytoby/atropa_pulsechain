#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define LANES 4
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define WARNING_DRONE  110
#define GRID_SIZE      8

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    char glyph;
} color_pixel_t;

// 200 Bytes or less parameter structure (Size: 24 bytes)
typedef struct {
    uint32_t seed;
    float phase_scale;
    float twirl_strength;
    uint8_t blend_mode;
    float light_x;
    float light_y;
    float light_z;
} texgen_params_t;

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

// Assembly-style 256-byte precalculated lookup table (LUT) for FPU-less fast sine/cosine
static uint8_t sin_table[256];
static bool lut_initialized = false;

static void init_sin_table(void) {
    for (int i = 0; i < 256; i++) {
        sin_table[i] = (uint8_t)(127.5 + 127.5 * sin(i * 2.0 * 3.141592653589793 / 256.0));
    }
    lut_initialized = true;
}

static double fast_sin(double angle) {
    if (!lut_initialized) init_sin_table();
    int idx = (int)(angle * 256.0 / (2.0 * 3.141592653589793));
    return ((double)sin_table[(idx % 256 + 256) % 256] - 127.5) / 127.5;
}

static double fast_cos(double angle) {
    if (!lut_initialized) init_sin_table();
    int idx = (int)(angle * 256.0 / (2.0 * 3.141592653589793)) + 64; // cos(x) = sin(x + pi/2)
    return ((double)sin_table[(idx % 256 + 256) % 256] - 127.5) / 127.5;
}

// Deterministic hash function for pseudo-random coordinates
static uint8_t hash_noise(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U + seed;
    h = (h ^ (h >> 13)) * 12741261U;
    return (uint8_t)(h & 0xFF);
}

// Cosine interpolation for smooth gradients using fast Cosine lookup
static double cosine_interpolate(double a, double b, double mu) {
    double mu2 = (1.0 - fast_cos(mu * 3.141592653589793)) / 2.0;
    return (a * (1.0 - mu2) + b * mu2);
}

// 2D Noise generator with Cosine Interpolation
static double noise_2d(double x, double y, uint32_t seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    double dx = x - x0;
    double dy = y - y0;
    
    double v00 = (double)hash_noise(x0, y0, seed) / 255.0;
    double v10 = (double)hash_noise(x1, y0, seed) / 255.0;
    double v01 = (double)hash_noise(x0, y1, seed) / 255.0;
    double v11 = (double)hash_noise(x1, y1, seed) / 255.0;
    
    double i1 = cosine_interpolate(v00, v10, dx);
    double i2 = cosine_interpolate(v01, v11, dx);
    
    return cosine_interpolate(i1, i2, dy);
}

// 1. Procedural Texture Synthesis (Layer-based 3-Channel TexGen with Twirl, Blending & Phong shading)
static void synthesize_texture_grid_rgb(huc_ocean_system_t *huc, double phase, int mode, const texgen_params_t *params) {
    color_pixel_t layer0[GRID_SIZE][GRID_SIZE]; // Base Plasma Layer (RGB channels)
    color_pixel_t layer1[GRID_SIZE][GRID_SIZE]; // Noise Distortion Map Layer (RGB channels)
    color_pixel_t layer_twirl[GRID_SIZE][GRID_SIZE];
    
    // Step 1: Generation - base plasma maps per channel
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            double p_val = 0.0;
            if (mode == 0) {
                // Conspiracy Directional Plasma using dynamic angle modulations
                double angle1 = phase * 0.1;
                double angle2 = phase * 0.25;
                p_val = 127.5 + 63.75 * fast_sin(x * fast_cos(angle1) + y * fast_sin(angle1)) +
                              63.75 * fast_cos(x * fast_sin(angle2) - y * fast_cos(angle2));
            } else if (mode == 1) {
                // Cross-directional modulated Sine Plasma
                double angle = phase * 0.15;
                p_val = 127.5 + 63.75 * fast_sin((x + y) * fast_cos(angle)) + 63.75 * fast_cos((x - y) * fast_sin(angle));
            } else if (mode == 2) {
                double f_val = 0.0;
                f_val += noise_2d((double)x * 0.25, (double)y * 0.25, params->seed) * 1.0;
                f_val += noise_2d((double)x * 0.5, (double)y * 0.5, params->seed + 100U) * 0.5;
                f_val += noise_2d((double)x * 1.0, (double)y * 1.0, params->seed + 200U) * 0.25;
                p_val = (f_val / 1.75) * 255.0;
            } else {
                double cx = (GRID_SIZE - 1) / 2.0;
                double cy = (GRID_SIZE - 1) / 2.0;
                double dx = x - cx;
                double dy = y - cy;
                double r = sqrt(dx*dx + dy*dy);
                double theta = atan2(dy, dx);
                double u = (theta + 3.141592653589793) / (2.0 * 3.141592653589793);
                double v = 1.0 / (r + 0.1);
                double u_scroll = u + phase * 0.05;
                double v_scroll = v + phase * 0.1;
                int check = ((int)(u_scroll * 12.0) % 2) ^ ((int)(v_scroll * 12.0) % 2);
                p_val = check ? 255.0 : 0.0;
            }
            
            // Assign distinct variations per channel
            layer0[y][x].r = (uint8_t)p_val;
            layer0[y][x].g = (uint8_t)(p_val * 0.7 + 30.0);
            layer0[y][x].b = (uint8_t)(p_val * 0.4 + 60.0);
        }
    }
    
    // Step 2: Generation - Noise layers
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            layer1[y][x].r = hash_noise(x, y, params->seed);
            layer1[y][x].g = hash_noise(x, y, params->seed + 50U);
            layer1[y][x].b = hash_noise(x, y, params->seed + 100U);
        }
    }
    
    // Step 3: Twirl Distortion & Multi-Layer Blending
    double cx = (GRID_SIZE - 1) / 2.0;
    double cy = (GRID_SIZE - 1) / 2.0;
    double twirl_strength = params->twirl_strength + fast_sin(phase) * 1.5;
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double r = sqrt(dx*dx + dy*dy);
            double theta = atan2(dy, dx);
            double theta_new = theta + twirl_strength * (1.0 - r / (GRID_SIZE * 0.707));
            
            double tx = cx + r * fast_cos(theta_new);
            double ty = cy + r * fast_sin(theta_new);
            
            int x0 = ((int)floor(tx) % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
            int x1 = (x0 + 1) % GRID_SIZE;
            int y0 = ((int)floor(ty) % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
            int y1 = (y0 + 1) % GRID_SIZE;
            
            double frac_x = tx - floor(tx);
            double frac_y = ty - floor(ty);
            
            double w00 = (1.0 - frac_x) * (1.0 - frac_y);
            double w10 = frac_x * (1.0 - frac_y);
            double w01 = (1.0 - frac_x) * frac_y;
            double w11 = frac_x * frac_y;
            
            // Apply twirl per channel
            layer_twirl[y][x].r = w00 * layer0[y0][x0].r + w10 * layer0[y0][x1].r + w01 * layer0[y1][x0].r + w11 * layer0[y1][x1].r;
            layer_twirl[y][x].g = w00 * layer0[y0][x0].g + w10 * layer0[y0][x1].g + w01 * layer0[y1][x0].g + w11 * layer0[y1][x1].g;
            layer_twirl[y][x].b = w00 * layer0[y0][x0].b + w10 * layer0[y0][x1].b + w01 * layer0[y1][x0].b + w11 * layer0[y1][x1].b;
            
            // Dynamic Blending Modes: Multiply (1) or Difference (2)
            if (params->blend_mode == 1) {
                layer_twirl[y][x].r = (uint8_t)((uint32_t)layer_twirl[y][x].r * layer1[y][x].r / 255U);
                layer_twirl[y][x].g = (uint8_t)((uint32_t)layer_twirl[y][x].g * layer1[y][x].g / 255U);
                layer_twirl[y][x].b = (uint8_t)((uint32_t)layer_twirl[y][x].b * layer1[y][x].b / 255U);
            } else {
                // Difference Blend Mode: abs(A - B)
                int dr = (int)layer_twirl[y][x].r - (int)layer1[y][x].r;
                int dg = (int)layer_twirl[y][x].g - (int)layer1[y][x].g;
                int db = (int)layer_twirl[y][x].b - (int)layer1[y][x].b;
                layer_twirl[y][x].r = (uint8_t)(dr < 0 ? -dr : dr);
                layer_twirl[y][x].g = (uint8_t)(dg < 0 ? -dg : dg);
                layer_twirl[y][x].b = (uint8_t)(db < 0 ? -db : db);
            }
        }
    }
    
    // Step 4: Bilinear Warp
    double warped_r[GRID_SIZE][GRID_SIZE];
    double warped_g[GRID_SIZE][GRID_SIZE];
    double warped_b[GRID_SIZE][GRID_SIZE];
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            double warp_x = (double)layer1[y][x].r / 255.0 * 2.0;
            double warp_y = (double)layer1[x][y].g / 255.0 * 2.0;
            
            double target_x = x + warp_x;
            double target_y = y + warp_y;
            
            int x0 = ((int)target_x) % GRID_SIZE;
            int x1 = (x0 + 1) % GRID_SIZE;
            int y0 = ((int)target_y) % GRID_SIZE;
            int y1 = (y0 + 1) % GRID_SIZE;
            
            double dx = target_x - (int)target_x;
            double dy = target_y - (int)target_y;
            
            double w00 = (1.0 - dx) * (1.0 - dy);
            double w10 = dx * (1.0 - dy);
            double w01 = (1.0 - dx) * dy;
            double w11 = dx * dy;
            
            warped_r[y][x] = w00 * layer_twirl[y0][x0].r + w10 * layer_twirl[y0][x1].r + w01 * layer_twirl[y1][x0].r + w11 * layer_twirl[y1][x1].r;
            warped_g[y][x] = w00 * layer_twirl[y0][x0].g + w10 * layer_twirl[y0][x1].g + w01 * layer_twirl[y1][x0].g + w11 * layer_twirl[y1][x1].g;
            warped_b[y][x] = w00 * layer_twirl[y0][x0].b + w10 * layer_twirl[y0][x1].b + w01 * layer_twirl[y1][x0].b + w11 * layer_twirl[y1][x1].b;
            
            // Waveform Modulators: Apply sine modulations to warp intensities
            warped_r[y][x] = 127.5 + 127.5 * fast_sin(warped_r[y][x] * 0.15);
            warped_g[y][x] = 127.5 + 127.5 * fast_sin(warped_g[y][x] * 0.15);
            warped_b[y][x] = 127.5 + 127.5 * fast_sin(warped_b[y][x] * 0.15);
        }
    }
    
    // Step 5: Bump normals & Phong Specular highlight mapping
    const char glyphs[] = " .:-=+*#%@";
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int x_prev = (x - 1 + GRID_SIZE) % GRID_SIZE;
            int x_next = (x + 1) % GRID_SIZE;
            int y_prev = (y - 1 + GRID_SIZE) % GRID_SIZE;
            int y_next = (y + 1) % GRID_SIZE;
            
            // Calculate normals using grayscale heightmap average
            double h_center = (warped_r[y][x] + warped_g[y][x] + warped_b[y][x]) / 3.0;
            double h_x_next = (warped_r[y][x_next] + warped_g[y][x_next] + warped_b[y][x_next]) / 3.0;
            double h_x_prev = (warped_r[y][x_prev] + warped_g[y][x_prev] + warped_b[y][x_prev]) / 3.0;
            double h_y_next = (warped_r[y_next][x] + warped_g[y_next][x] + warped_b[y_next][x]) / 3.0;
            double h_y_prev = (warped_r[y_prev][x] + warped_g[y_prev][x] + warped_b[y_prev][x]) / 3.0;
            
            double dh_dx = (h_x_next - h_x_prev) / 2.0;
            double dh_dy = (h_y_next - h_y_prev) / 2.0;
            
            double nx = -dh_dx;
            double ny = -dh_dy;
            double nz = 32.0;
            double len = sqrt(nx*nx + ny*ny + nz*nz);
            nx /= len; ny /= len; nz /= len;
            
            // Phong specular light calculations using pre-normalized vector (4KB style)
            double lx = params->light_x; 
            double ly = params->light_y; 
            double lz = params->light_z;
            
            double diffuse = nx*lx + ny*ly + nz*lz;
            if (diffuse < 0.0) diffuse = 0.0;
            
            double rz = 2.0 * diffuse * nz - lz;
            double spec = rz;
            if (spec < 0.0) spec = 0.0;
            
            // Fast squaring shininess exponent
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            
            double combined_light = diffuse * 0.65 + spec * 0.35;
            if (combined_light > 1.0) combined_light = 1.0;
            
            if (mode % 2 == 0) {
                // Normal Map color mode modulated by light intensity
                huc->texture_grid[y][x].r = (uint8_t)(((nx + 1.0) * 127.5) * combined_light);
                huc->texture_grid[y][x].g = (uint8_t)(((ny + 1.0) * 127.5) * combined_light);
                huc->texture_grid[y][x].b = (uint8_t)(((nz + 1.0) * 127.5) * combined_light);
            } else {
                // Gradient Map color mode modulated by light intensity
                double intensity = h_center / 255.0 * combined_light;
                if (intensity < 0.5) {
                    double t = intensity * 2.0;
                    huc->texture_grid[y][x].r = (uint8_t)(2.0 + t * 12.0);
                    huc->texture_grid[y][x].g = (uint8_t)(8.0 + t * 157.0);
                    huc->texture_grid[y][x].b = (uint8_t)(19.0 + t * 214.0);
                } else {
                    double t = (intensity - 0.5) * 2.0;
                    huc->texture_grid[y][x].r = (uint8_t)(14.0 + t * 241.0);
                    huc->texture_grid[y][x].g = (uint8_t)(165.0 + t * 90.0);
                    huc->texture_grid[y][x].b = (uint8_t)(233.0 + t * 22.0);
                }
            }
            
            int idx = (int)((h_center * combined_light) / 25.6);
            if (idx < 0) idx = 0;
            if (idx > 9) idx = 9;
            huc->texture_grid[y][x].glyph = glyphs[idx];
        }
    }
}

// 2. Procedural 3D Mesh / Lissajous Pathing
static void update_lissajous_mesh(huc_ocean_system_t *huc, double signal) {
    huc->lissajous_x = fast_sin(signal * 2.5);
    huc->lissajous_y = fast_cos(signal * 1.8);
    huc->lissajous_z = fast_sin(signal * 3.2 + 0.5);
}

// 3. Low-Level Tape Ingest process
static void process_tape_ingest_v6(huc_ocean_system_t *huc,
                                   const sgpr_bank_t *sgprs,
                                   vgpr_bank_t *vgprs) {
    printf("[PROCESS] Executing Level 6 Hogan-Hudson Ocean Tape Ingest...\n");
    fflush(stdout);

    // Initialized parameter struct under 200 bytes limit
    texgen_params_t params = {
        .seed = 999U,
        .phase_scale = 1.5f,
        .twirl_strength = 2.0f,
        .blend_mode = 1U,
        .light_x = 0.57735f,
        .light_y = 0.57735f,
        .light_z = 0.57735f
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

        double lfo_mod = fast_sin(huc->transaction_count * 0.8) * 50.0;
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
