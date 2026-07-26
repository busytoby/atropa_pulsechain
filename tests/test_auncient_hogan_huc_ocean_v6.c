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
    uint8_t r;
    uint8_t g;
    uint8_t b;
    char glyph;
} color_pixel_t;

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

// Deterministic hash function for pseudo-random coordinates
static uint8_t hash_noise(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U + seed;
    h = (h ^ (h >> 13)) * 12741261U;
    return (uint8_t)(h & 0xFF);
}

// Cosine interpolation for smooth gradients
static double cosine_interpolate(double a, double b, double mu) {
    double mu2 = (1.0 - cos(mu * 3.141592653589793)) / 2.0;
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

// 1. Procedural Texture Synthesis (Layer-based TexGen with Bilinear Distortion)
static void synthesize_texture_grid(huc_ocean_system_t *huc, double phase, int mode) {
    uint8_t layer0[GRID_SIZE][GRID_SIZE]; // Base Plasma Layer
    uint8_t layer1[GRID_SIZE][GRID_SIZE]; // Noise Distortion Map Layer
    
    // Step 1: Generate Base Plasma Layer based on selected mode
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            double val = 0.0;
            if (mode == 0) {
                // Standard Multi-Component Plasma
                val = 127.5 + 42.5 * sin(x * 1.0 + phase) +
                              42.5 * sin(y * 1.5 - phase) +
                              42.5 * sin((x + y) * 0.8 + phase);
            } else if (mode == 1) {
                // Sine Plasma
                val = 127.5 + 63.75 * sin(x * 1.2 + phase) + 63.75 * cos(y * 1.2 + phase * 0.8);
            } else if (mode == 2) {
                // Fractal Plasma (Multi-Octave Perlin Noise with Cosine Interpolation)
                double f_val = 0.0;
                f_val += noise_2d((double)x * 0.25, (double)y * 0.25, 100U) * 1.0;
                f_val += noise_2d((double)x * 0.5, (double)y * 0.5, 200U) * 0.5;
                f_val += noise_2d((double)x * 1.0, (double)y * 1.0, 300U) * 0.25;
                val = (f_val / 1.75) * 255.0;
            } else {
                // 3D Swirling Passage Tunnel (Inverse distance and angle mapping)
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
                val = check ? 255.0 : 0.0;
            }
            layer0[y][x] = (uint8_t)val;
        }
    }
    
    // Step 2: Apply Twirl Distortion to Base Plasma Layer
    uint8_t layer_twirl[GRID_SIZE][GRID_SIZE];
    double cx = (GRID_SIZE - 1) / 2.0;
    double cy = (GRID_SIZE - 1) / 2.0;
    double twirl_strength = 2.0 + sin(phase) * 1.5; // Dynamic twirl strength
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double r = sqrt(dx*dx + dy*dy);
            double theta = atan2(dy, dx);
            
            // Add twist proportional to distance
            double theta_new = theta + twirl_strength * (1.0 - r / (GRID_SIZE * 0.707));
            
            double tx = cx + r * cos(theta_new);
            double ty = cy + r * sin(theta_new);
            
            // Bilinear interpolation bounds for twirl mapping
            int x0 = ((int)floor(tx) % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
            int x1 = (x0 + 1) % GRID_SIZE;
            int y0 = ((int)floor(ty) % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
            int y1 = (y0 + 1) % GRID_SIZE;
            
            double frac_x = tx - floor(tx);
            double frac_y = ty - floor(ty);
            
            double val = (1.0 - frac_x) * (1.0 - frac_y) * layer0[y0][x0] +
                         frac_x * (1.0 - frac_y) * layer0[y0][x1] +
                         (1.0 - frac_x) * frac_y * layer0[y1][x0] +
                         frac_x * frac_y * layer0[y1][x1];
                         
            layer_twirl[y][x] = (uint8_t)val;
        }
    }
    
    // Step 3: Generate Deterministic Noise Map on Layer 1 and Multiply Blend
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            layer1[y][x] = hash_noise(x, y, 999U);
            
            // Multiply Blend Mode: Blend base plasma with noise map
            uint32_t blended = (uint32_t)layer_twirl[y][x] * (uint32_t)layer1[y][x] / 255U;
            layer_twirl[y][x] = (uint8_t)blended;
        }
    }
    
    // Step 4: Warp Layer Twirl using Layer 1 and apply Sine Waveform Modulator
    double warped_intensity[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            // Read warp displacements from layer 1
            double warp_x = (double)layer1[y][x] / 255.0 * 2.0; // range 0 to 2
            double warp_y = (double)layer1[x][y] / 255.0 * 2.0; // range 0 to 2
            
            double target_x = x + warp_x;
            double target_y = y + warp_y;
            
            // Bilinear interpolation bounds for warp mapping
            int x0 = ((int)target_x) % GRID_SIZE;
            int x1 = (x0 + 1) % GRID_SIZE;
            int y0 = ((int)target_y) % GRID_SIZE;
            int y1 = (y0 + 1) % GRID_SIZE;
            
            double dx = target_x - (int)target_x;
            double dy = target_y - (int)target_y;
            
            // Interpolate weights
            double w00 = (1.0 - dx) * (1.0 - dy);
            double w10 = dx * (1.0 - dy);
            double w01 = (1.0 - dx) * dy;
            double w11 = dx * dy;
            
            double interpolated = w00 * layer_twirl[y0][x0] +
                                  w10 * layer_twirl[y0][x1] +
                                  w01 * layer_twirl[y1][x0] +
                                  w11 * layer_twirl[y1][x1];
                                  
            // Sine Waveform Modulator: Map linear gradients to wood/wave rings
            warped_intensity[y][x] = 127.5 + 127.5 * sin(interpolated * 0.15);
        }
    }
    
    // Step 5: Compute bump normals and map pixel color gradients
    const char glyphs[] = " .:-=+*#%@";
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int x_prev = (x - 1 + GRID_SIZE) % GRID_SIZE;
            int x_next = (x + 1) % GRID_SIZE;
            int y_prev = (y - 1 + GRID_SIZE) % GRID_SIZE;
            int y_next = (y + 1) % GRID_SIZE;
            
            // Height-map normal calculation
            double dh_dx = (warped_intensity[y][x_next] - warped_intensity[y][x_prev]) / 2.0;
            double dh_dy = (warped_intensity[y_next][x] - warped_intensity[y_prev][x]) / 2.0;
            
            double nx = -dh_dx;
            double ny = -dh_dy;
            double nz = 32.0; // Scaling factor
            double len = sqrt(nx*nx + ny*ny + nz*nz);
            nx /= len;
            ny /= len;
            nz /= len;
            
            // Phong specular light calculations
            double lx = 1.0, ly = 1.0, lz = 1.5;
            double llen = sqrt(lx*lx + ly*ly + lz*lz);
            lx /= llen; ly /= llen; lz /= llen;
            
            double diffuse = nx*lx + ny*ly + nz*lz;
            if (diffuse < 0.0) diffuse = 0.0;
            
            double rz = 2.0 * diffuse * nz - lz;
            
            double spec = rz; // dot(R, V) where V = (0, 0, 1)
            if (spec < 0.0) spec = 0.0;
            spec = pow(spec, 16.0); // Shininess exponent
            
            // Combine diffuse and specular light components
            double combined_light = diffuse * 0.65 + spec * 0.35;
            if (combined_light > 1.0) combined_light = 1.0;
            
            if (mode % 2 == 0) {
                // Normal Map mapping modulated by light intensity
                huc->texture_grid[y][x].r = (uint8_t)(((nx + 1.0) * 127.5) * combined_light);
                huc->texture_grid[y][x].g = (uint8_t)(((ny + 1.0) * 127.5) * combined_light);
                huc->texture_grid[y][x].b = (uint8_t)(((nz + 1.0) * 127.5) * combined_light);
            } else {
                // Gradient Map mapping modulated by light intensity
                double intensity = warped_intensity[y][x] / 255.0 * combined_light;
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
            
            int idx = (int)((warped_intensity[y][x] * combined_light) / 25.6);
            if (idx < 0) idx = 0;
            if (idx > 9) idx = 9;
            huc->texture_grid[y][x].glyph = glyphs[idx];
        }
    }
}

// 2. Procedural 3D Mesh / Lissajous Pathing
static void update_lissajous_mesh(huc_ocean_system_t *huc, double signal) {
    // Standard demoscene harmonic rotation formula
    huc->lissajous_x = sin(signal * 2.5);
    huc->lissajous_y = cos(signal * 1.8);
    huc->lissajous_z = sin(signal * 3.2 + 0.5);
}

// 3. Low-Level Tape Ingest process with Conspiracy Demoscene integrations
static void process_tape_ingest_v6(huc_ocean_system_t *huc,
                                   const sgpr_bank_t *sgprs,
                                   vgpr_bank_t *vgprs) {
    printf("[PROCESS] Executing Level 6 Hogan-Hudson Ocean Tape Ingest...\n");
    fflush(stdout);

    for (int lane = 0; lane < LANES; lane++) {
        uint32_t acc = vgprs->account_id[lane];
        uint64_t votes = vgprs->accumulated_votes[lane];
        
        // Default to black SGPR baseline color
        huc->border_color = sgprs->baseline_color;

        // 1. Check Transaction Count Limit
        if (huc->transaction_count >= sgprs->max_allowed_transactions) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED; // Red VGPR alert color
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [INGEST FAIL] Lane %d: Transaction count ceiling exceeded! Ingest aborted.\n", lane);
            fflush(stdout);
            break;
        }

        // 2. Check Accumulator Quorum Threshold
        if (votes < sgprs->consensus_threshold) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED; // Red VGPR alert color due to failed quorum threshold
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [INGEST FAIL] Lane %d: Account %d lacks sufficient quorum votes (%lu < %lu)! Ingest aborted.\n", 
                   lane, acc, votes, sgprs->consensus_threshold);
            fflush(stdout);
            break;
        }

        // Success: Process transaction
        huc->transaction_count++;
        vgprs->status[lane] = COLOR_CYAN;
        huc->border_color = COLOR_CYAN;

        // Conspiracy LFO Modulator: Dynamic sound synthesis utilizing a Low-Frequency Oscillator
        double lfo_mod = sin(huc->transaction_count * 0.8) * 50.0;
        huc->psg_frequency = (uint32_t)(261.0 + (huc->transaction_count * 20.0) + lfo_mod);

        // Update procedural visualizers
        synthesize_texture_grid(huc, huc->transaction_count * 1.5, lane % 4);
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

    // SGPR settings: limit of 2 transactions, consensus threshold of 600
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
            600, // Lane 0: Pass (meets threshold)
            600, // Lane 1: Pass (meets threshold)
            300, // Lane 2: Fail (below threshold)
            600  // Lane 3: Fail (would meet threshold, but count ceiling hit)
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

    // Process Level 6 tape ingest
    process_tape_ingest_v6(&huc, &sgprs, &vgprs);

    // Verify assertions
    printf("[TEST] Verifying combined system states...\n");
    fflush(stdout);

    // Lane 0: Pass
    assert(vgprs.status[0] == COLOR_CYAN);
    
    // Lane 1: Pass
    assert(vgprs.status[1] == COLOR_CYAN);
    
    // Lane 2: Fail (Quorum votes 300 < 600)
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
