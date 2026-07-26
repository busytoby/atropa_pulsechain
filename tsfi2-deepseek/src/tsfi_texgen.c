#include "tsfi_texgen.h"
#include <math.h>
#include <stdlib.h>

static uint8_t sin_table[256];
static bool lut_initialized = false;

void tsfi_texgen_init(void) {
    if (lut_initialized) return;
    for (int i = 0; i < 256; i++) {
        sin_table[i] = (uint8_t)(127.5 + 127.5 * sin(i * 2.0 * 3.141592653589793 / 256.0));
    }
    lut_initialized = true;
}

double tsfi_texgen_sin(double angle) {
    if (!lut_initialized) tsfi_texgen_init();
    int idx = (int)(angle * 256.0 / (2.0 * 3.141592653589793));
    return ((double)sin_table[(idx % 256 + 256) % 256] - 127.5) / 127.5;
}

double tsfi_texgen_cos(double angle) {
    if (!lut_initialized) tsfi_texgen_init();
    int idx = (int)(angle * 256.0 / (2.0 * 3.141592653589793)) + 64;
    return ((double)sin_table[(idx % 256 + 256) % 256] - 127.5) / 127.5;
}

static uint8_t hash_noise(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U + seed;
    h = (h ^ (h >> 13)) * 12741261U;
    return (uint8_t)(h & 0xFF);
}

static double cosine_interpolate(double a, double b, double mu) {
    double mu2 = (1.0 - tsfi_texgen_cos(mu * 3.141592653589793)) / 2.0;
    return (a * (1.0 - mu2) + b * mu2);
}

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

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_pix_t;

void tsfi_texgen_render(uint8_t *output_rgba, int width, int height, double phase, int mode, const tsfi_texgen_params_t *params) {
    if (!lut_initialized) tsfi_texgen_init();
    
    rgb_pix_t *layer0 = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    rgb_pix_t *layer1 = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    rgb_pix_t *layer_twirl = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    double *warped_r = (double *)malloc(width * height * sizeof(double));
    double *warped_g = (double *)malloc(width * height * sizeof(double));
    double *warped_b = (double *)malloc(width * height * sizeof(double));
    
    // Step 1: Base Plasma generation with dynamic lacunarity / persistence noise octaves, Voronoi, and Marble
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double p_val = 0.0;
            if (mode == 0) {
                double angle1 = phase * 0.1;
                double angle2 = phase * 0.25;
                p_val = 127.5 + 63.75 * tsfi_texgen_sin(x * tsfi_texgen_cos(angle1) + y * tsfi_texgen_sin(angle1)) +
                              63.75 * tsfi_texgen_cos(x * tsfi_texgen_sin(angle2) - y * tsfi_texgen_cos(angle2));
            } else if (mode == 1) {
                double angle = phase * 0.15;
                p_val = 127.5 + 63.75 * tsfi_texgen_sin((x + y) * tsfi_texgen_cos(angle)) +
                              63.75 * tsfi_texgen_cos((x - y) * tsfi_texgen_sin(angle));
            } else if (mode == 2) {
                double f_val = 0.0;
                double freq = 0.25;
                double amp = 1.0;
                double max_amp = 0.0;
                for (int o = 0; o < params->octaves; o++) {
                    f_val += noise_2d((double)x * freq, (double)y * freq, params->seed + o * 100U) * amp;
                    max_amp += amp;
                    freq *= params->lacunarity;
                    amp *= params->persistence;
                }
                p_val = (f_val / max_amp) * 255.0;
            } else if (mode == 3) {
                double cx = (width - 1) / 2.0;
                double cy = (height - 1) / 2.0;
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
            } else if (mode == 4) {
                // Voronoi Cellular Noise (Worley Noise)
                double min_dist = 999.0;
                for (int cell_y = -2; cell_y <= 2; cell_y++) {
                    for (int cell_x = -2; cell_x <= 2; cell_x++) {
                        int grid_cx = ((x + cell_x) % width + width) % width;
                        int grid_cy = ((y + cell_y) % height + height) % height;
                        double px = grid_cx + (double)hash_noise(grid_cx, grid_cy, params->seed) / 255.0;
                        double py = grid_cy + (double)hash_noise(grid_cx, grid_cy, params->seed + 50U) / 255.0;
                        double dx = x - px;
                        double dy = y - py;
                        double dist = sqrt(dx*dx + dy*dy);
                        if (dist < min_dist) min_dist = dist;
                    }
                }
                p_val = min_dist * 80.0;
                if (p_val > 255.0) p_val = 255.0;
            } else {
                // Turbulence Marble pattern: sin(x + turbulence * noise)
                double f_val = 0.0;
                double freq = 0.25;
                double amp = 1.0;
                double max_amp = 0.0;
                for (int o = 0; o < params->octaves; o++) {
                    f_val += noise_2d((double)x * freq, (double)y * freq, params->seed + o * 100U) * amp;
                    max_amp += amp;
                    freq *= params->lacunarity;
                    amp *= params->persistence;
                }
                double noise_val = (f_val / max_amp);
                p_val = 127.5 + 127.5 * tsfi_texgen_sin(x * 0.15 + noise_val * 10.0);
            }
            
            int idx = y * width + x;
            layer0[idx].r = (uint8_t)p_val;
            layer0[idx].g = (uint8_t)(p_val * 0.7 + 30.0);
            layer0[idx].b = (uint8_t)(p_val * 0.4 + 60.0);
        }
    }
    
    // Step 2: Noise generation
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            layer1[idx].r = hash_noise(x, y, params->seed);
            layer1[idx].g = hash_noise(x, y, params->seed + 50U);
            layer1[idx].b = hash_noise(x, y, params->seed + 100U);
        }
    }
    
    // Step 3: Twirl & Blending mode options (Multiply, Difference, Screen, Add, Subtract)
    double cx = (width - 1) / 2.0;
    double cy = (height - 1) / 2.0;
    double twirl_strength = params->twirl_strength + tsfi_texgen_sin(phase) * 1.5;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double r = sqrt(dx*dx + dy*dy);
            double theta = atan2(dy, dx);
            double theta_new = theta + twirl_strength * (1.0 - r / (width * 0.707));
            
            double tx = cx + r * tsfi_texgen_cos(theta_new);
            double ty = cy + r * tsfi_texgen_sin(theta_new);
            
            int x0 = ((int)floor(tx) % width + width) % width;
            int x1 = (x0 + 1) % width;
            int y0 = ((int)floor(ty) % height + height) % height;
            int y1 = (y0 + 1) % height;
            
            double frac_x = tx - floor(tx);
            double frac_y = ty - floor(ty);
            
            double w00 = (1.0 - frac_x) * (1.0 - frac_y);
            double w10 = frac_x * (1.0 - frac_y);
            double w01 = (1.0 - frac_x) * frac_y;
            double w11 = frac_x * frac_y;
            
            int idx = y * width + x;
            layer_twirl[idx].r = w00 * layer0[y0*width + x0].r + w10 * layer0[y0*width + x1].r + w01 * layer0[y1*width + x0].r + w11 * layer0[y1*width + x1].r;
            layer_twirl[idx].g = w00 * layer0[y0*width + x0].g + w10 * layer0[y0*width + x1].g + w01 * layer0[y1*width + x0].g + w11 * layer0[y1*width + x1].g;
            layer_twirl[idx].b = w00 * layer0[y0*width + x0].b + w10 * layer0[y0*width + x1].b + w01 * layer0[y1*width + x0].b + w11 * layer0[y1*width + x1].b;
            
            if (params->blend_mode == 1) { // Multiply
                layer_twirl[idx].r = (uint8_t)((uint32_t)layer_twirl[idx].r * layer1[idx].r / 255U);
                layer_twirl[idx].g = (uint8_t)((uint32_t)layer_twirl[idx].g * layer1[idx].g / 255U);
                layer_twirl[idx].b = (uint8_t)((uint32_t)layer_twirl[idx].b * layer1[idx].b / 255U);
            } else if (params->blend_mode == 2) { // Difference
                int dr = (int)layer_twirl[idx].r - (int)layer1[idx].r;
                int dg = (int)layer_twirl[idx].g - (int)layer1[idx].g;
                int db = (int)layer_twirl[idx].b - (int)layer1[idx].b;
                layer_twirl[idx].r = (uint8_t)(dr < 0 ? -dr : dr);
                layer_twirl[idx].g = (uint8_t)(dg < 0 ? -dg : dg);
                layer_twirl[idx].b = (uint8_t)(db < 0 ? -db : db);
            } else if (params->blend_mode == 3) { // Screen
                layer_twirl[idx].r = (uint8_t)(255 - ((255 - layer_twirl[idx].r) * (255 - layer1[idx].r) / 255));
                layer_twirl[idx].g = (uint8_t)(255 - ((255 - layer_twirl[idx].g) * (255 - layer1[idx].g) / 255));
                layer_twirl[idx].b = (uint8_t)(255 - ((255 - layer_twirl[idx].b) * (255 - layer1[idx].b) / 255));
            } else if (params->blend_mode == 4) { // Add
                uint32_t r_add = (uint32_t)layer_twirl[idx].r + layer1[idx].r;
                uint32_t g_add = (uint32_t)layer_twirl[idx].g + layer1[idx].g;
                uint32_t b_add = (uint32_t)layer_twirl[idx].b + layer1[idx].b;
                layer_twirl[idx].r = r_add > 255 ? 255 : r_add;
                layer_twirl[idx].g = g_add > 255 ? 255 : g_add;
                layer_twirl[idx].b = b_add > 255 ? 255 : b_add;
            } else { // Subtract
                int r_sub = (int)layer_twirl[idx].r - layer1[idx].r;
                int g_sub = (int)layer_twirl[idx].g - layer1[idx].g;
                int b_sub = (int)layer_twirl[idx].b - layer1[idx].b;
                layer_twirl[idx].r = r_sub < 0 ? 0 : r_sub;
                layer_twirl[idx].g = g_sub < 0 ? 0 : g_sub;
                layer_twirl[idx].b = b_sub < 0 ? 0 : b_sub;
            }
        }
    }
    
    // Step 4: Bilinear Warp & Waveform modulation & feedback recursion loop & Sine coordinate ripple warp
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            
            double ripple_x = tsfi_texgen_sin(y * 0.2 + phase) * 2.0;
            double ripple_y = tsfi_texgen_cos(x * 0.2 + phase) * 2.0;
            
            double warp_x = (double)layer1[idx].r / 255.0 * 2.0 + params->feedback_strength * tsfi_texgen_sin(x * 0.1) + ripple_x;
            double warp_y = (double)layer1[(x % width) * width + (y % height)].g / 255.0 * 2.0 + params->feedback_strength * tsfi_texgen_cos(y * 0.1) + ripple_y;
            
            double target_x = x + warp_x;
            double target_y = y + warp_y;
            
            int x0 = ((int)target_x) % width;
            int x1 = (x0 + 1) % width;
            int y0 = ((int)target_y) % height;
            int y1 = (y0 + 1) % height;
            
            double dx = target_x - (int)target_x;
            double dy = target_y - (int)target_y;
            
            double w00 = (1.0 - dx) * (1.0 - dy);
            double w10 = dx * (1.0 - dy);
            double w01 = (1.0 - dx) * dy;
            double w11 = dx * dy;
            
            double r_val = w00 * layer_twirl[y0*width + x0].r + w10 * layer_twirl[y0*width + x1].r + w01 * layer_twirl[y1*width + x0].r + w11 * layer_twirl[y1*width + x1].r;
            double g_val = w00 * layer_twirl[y0*width + x0].g + w10 * layer_twirl[y0*width + x1].g + w01 * layer_twirl[y1*width + x0].g + w11 * layer_twirl[y1*width + x1].g;
            double b_val = w00 * layer_twirl[y0*width + x0].b + w10 * layer_twirl[y0*width + x1].b + w01 * layer_twirl[y1*width + x0].b + w11 * layer_twirl[y1*width + x1].b;
            
            if (params->wave_type == 0) { // Sine waveform
                warped_r[idx] = 127.5 + 127.5 * tsfi_texgen_sin(r_val * 0.15);
                warped_g[idx] = 127.5 + 127.5 * tsfi_texgen_sin(g_val * 0.15);
                warped_b[idx] = 127.5 + 127.5 * tsfi_texgen_sin(b_val * 0.15);
            } else if (params->wave_type == 1) { // Sawtooth waveform
                double rs = (r_val * 0.05);
                double gs = (g_val * 0.05);
                double bs = (b_val * 0.05);
                warped_r[idx] = (rs - floor(rs)) * 255.0;
                warped_g[idx] = (gs - floor(gs)) * 255.0;
                warped_b[idx] = (bs - floor(bs)) * 255.0;
            } else { // Square waveform
                warped_r[idx] = (tsfi_texgen_sin(r_val * 0.15) >= 0.0) ? 255.0 : 0.0;
                warped_g[idx] = (tsfi_texgen_sin(g_val * 0.15) >= 0.0) ? 255.0 : 0.0;
                warped_b[idx] = (tsfi_texgen_sin(b_val * 0.15) >= 0.0) ? 255.0 : 0.0;
            }
        }
    }
    
    // Step 5: Sobel filter normal calculations, Anisotropic spec shading, Ambient occlusion & RGBA packing
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int x_prev = (x - 1 + width) % width;
            int x_next = (x + 1) % width;
            int y_prev = (y - 1 + height) % height;
            int y_next = (y + 1) % height;
            
            int idx = y * width + x;
            
            double h00 = (warped_r[y_prev*width + x_prev] + warped_g[y_prev*width + x_prev] + warped_b[y_prev*width + x_prev]) / 3.0;
            double h01 = (warped_r[y_prev*width + x] + warped_g[y_prev*width + x] + warped_b[y_prev*width + x]) / 3.0;
            double h02 = (warped_r[y_prev*width + x_next] + warped_g[y_prev*width + x_next] + warped_b[y_prev*width + x_next]) / 3.0;
            
            double h10 = (warped_r[y*width + x_prev] + warped_g[y*width + x_prev] + warped_b[y*width + x_prev]) / 3.0;
            double h12 = (warped_r[y*width + x_next] + warped_g[y*width + x_next] + warped_b[y*width + x_next]) / 3.0;
            
            double h20 = (warped_r[y_next*width + x_prev] + warped_g[y_next*width + x_prev] + warped_b[y_next*width + x_prev]) / 3.0;
            double h21 = (warped_r[y_next*width + x] + warped_g[y_next*width + x] + warped_b[y_next*width + x]) / 3.0;
            double h22 = (warped_r[y_next*width + x_next] + warped_g[y_next*width + x_next] + warped_b[y_next*width + x_next]) / 3.0;
            
            double dh_dx = (h02 + 2.0*h12 + h22 - h00 - 2.0*h10 - h20) / 8.0;
            double dh_dy = (h20 + 2.0*h21 + h22 - h00 - 2.0*h01 - h02) / 8.0;
            
            double nx = -dh_dx;
            double ny = -dh_dy;
            double nz = 32.0;
            double len = sqrt(nx*nx + ny*ny + nz*nz);
            nx /= len; ny /= len; nz /= len;
            
            double lx = params->light_x;
            double ly = params->light_y;
            double lz = params->light_z;
            
            double diffuse = nx*lx + ny*ly + nz*lz;
            if (diffuse < 0.0) diffuse = 0.0;
            
            // Standard isotropic specular reflection
            double rz = 2.0 * diffuse * nz - lz;
            double spec = rz;
            if (spec < 0.0) spec = 0.0;
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            
            // Kajiya-Kay Anisotropic shading model approximation for strands
            double tx = ny;
            double ty = -nx;
            double tz = 0.0;
            double tlen = sqrt(tx*tx + ty*ty);
            if (tlen > 0.0) { tx /= tlen; ty /= tlen; }
            double t_dot_l = tx*lx + ty*ly + tz*lz;
            double sin_tl = sqrt(1.0 - t_dot_l*t_dot_l);
            double spec_aniso = sin_tl;
            if (spec_aniso < 0.0) spec_aniso = 0.0;
            spec_aniso = spec_aniso * spec_aniso;
            spec_aniso = spec_aniso * spec_aniso;
            spec_aniso = spec_aniso * spec_aniso;
            spec_aniso = spec_aniso * spec_aniso;
            
            double final_spec = (1.0 - params->anisotropy) * spec + params->anisotropy * spec_aniso;
            
            // Ambient Occlusion depth factor
            double ao_factor = 1.0 - (params->shell_ao * (1.0 / (1.0 + phase)));
            if (ao_factor < 0.0) ao_factor = 0.0;
            if (ao_factor > 1.0) ao_factor = 1.0;
            
            double combined_light = (diffuse * 0.65 + final_spec * 0.35) * ao_factor;
            if (combined_light > 1.0) combined_light = 1.0;
            
            int out_idx = idx * 4;
            double raw_r = 0.0, raw_g = 0.0, raw_b = 0.0;
            
            double h_center = ((warped_r[idx] + warped_g[idx] + warped_b[idx]) / 3.0);
            
            double intensity = h_center / 255.0 * combined_light;
            
            if (params->clut_preset == 0) {
                if (intensity < 0.5) {
                    double t = intensity * 2.0;
                    raw_r = 2.0 + t * 12.0;
                    raw_g = 8.0 + t * 157.0;
                    raw_b = 19.0 + t * 214.0;
                } else {
                    double t = (intensity - 0.5) * 2.0;
                    raw_r = 14.0 + t * 241.0;
                    raw_g = 165.0 + t * 90.0;
                    raw_b = 233.0 + t * 22.0;
                }
            } else if (params->clut_preset == 1) {
                if (intensity < 0.33) {
                    double t = intensity * 3.0;
                    raw_r = t * 255.0;
                    raw_g = 0.0;
                    raw_b = 0.0;
                } else if (intensity < 0.66) {
                    double t = (intensity - 0.33) * 3.0;
                    raw_r = 255.0;
                    raw_g = t * 255.0;
                    raw_b = 0.0;
                } else {
                    double t = (intensity - 0.66) * 3.0;
                    raw_r = 255.0;
                    raw_g = 255.0;
                    raw_b = t * 255.0;
                }
            } else if (params->clut_preset == 2) {
                raw_r = intensity * 180.0 + 50.0;
                raw_g = intensity * 50.0;
                raw_b = intensity * 255.0;
            } else {
                raw_r = intensity * 212.0;
                raw_g = intensity * 175.0;
                raw_b = intensity * 55.0;
            }
            
            double adj_r = (raw_r - 128.0) * params->contrast + 128.0 + params->brightness;
            double adj_g = (raw_g - 128.0) * params->contrast + 128.0 + params->brightness;
            double adj_b = (raw_b - 128.0) * params->contrast + 128.0 + params->brightness;
            
            double dx_vig = (double)x / width - 0.5;
            double dy_vig = (double)y / height - 0.5;
            double r_vig = sqrt(dx_vig*dx_vig + dy_vig*dy_vig);
            double vig_factor = 1.0 - params->vignette_strength * (r_vig * 1.414);
            if (vig_factor < 0.0) vig_factor = 0.0;
            
            adj_r *= vig_factor;
            adj_g *= vig_factor;
            adj_b *= vig_factor;
            
            output_rgba[out_idx]   = adj_r < 0.0 ? 0 : (adj_r > 255.0 ? 255 : (uint8_t)adj_r);
            output_rgba[out_idx+1] = adj_g < 0.0 ? 0 : (adj_g > 255.0 ? 255 : (uint8_t)adj_g);
            output_rgba[out_idx+2] = adj_b < 0.0 ? 0 : (adj_b > 255.0 ? 255 : (uint8_t)adj_b);
            
            // RGBA Alpha threshold masking mapping
            uint8_t alpha = 255;
            if (params->threshold_limit > 0.0f) {
                if (h_center < params->threshold_limit) {
                    alpha = 0;
                } else {
                    double delta = (h_center - params->threshold_limit) / (255.0 - params->threshold_limit);
                    alpha = (uint8_t)(delta * 255.0);
                }
            }
            output_rgba[out_idx+3] = alpha;
        }
    }
    
    free(layer0);
    free(layer1);
    free(layer_twirl);
    free(warped_r);
    free(warped_g);
    free(warped_b);
}
