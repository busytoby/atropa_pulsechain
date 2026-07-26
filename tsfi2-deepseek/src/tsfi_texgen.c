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

void tsfi_texgen_render(uint8_t *output_rgb, int width, int height, double phase, int mode, const tsfi_texgen_params_t *params) {
    if (!lut_initialized) tsfi_texgen_init();
    
    // Allocate temporary layers dynamically based on width/height
    rgb_pix_t *layer0 = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    rgb_pix_t *layer1 = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    rgb_pix_t *layer_twirl = (rgb_pix_t *)malloc(width * height * sizeof(rgb_pix_t));
    double *warped_r = (double *)malloc(width * height * sizeof(double));
    double *warped_g = (double *)malloc(width * height * sizeof(double));
    double *warped_b = (double *)malloc(width * height * sizeof(double));
    
    // Step 1: Base Plasma generation
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
                f_val += noise_2d((double)x * 0.25, (double)y * 0.25, params->seed) * 1.0;
                f_val += noise_2d((double)x * 0.5, (double)y * 0.5, params->seed + 100U) * 0.5;
                f_val += noise_2d((double)x * 1.0, (double)y * 1.0, params->seed + 200U) * 0.25;
                p_val = (f_val / 1.75) * 255.0;
            } else {
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
    
    // Step 3: Twirl & Blend
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
            
            if (params->blend_mode == 1) {
                layer_twirl[idx].r = (uint8_t)((uint32_t)layer_twirl[idx].r * layer1[idx].r / 255U);
                layer_twirl[idx].g = (uint8_t)((uint32_t)layer_twirl[idx].g * layer1[idx].g / 255U);
                layer_twirl[idx].b = (uint8_t)((uint32_t)layer_twirl[idx].b * layer1[idx].b / 255U);
            } else {
                int dr = (int)layer_twirl[idx].r - (int)layer1[idx].r;
                int dg = (int)layer_twirl[idx].g - (int)layer1[idx].g;
                int db = (int)layer_twirl[idx].b - (int)layer1[idx].b;
                layer_twirl[idx].r = (uint8_t)(dr < 0 ? -dr : dr);
                layer_twirl[idx].g = (uint8_t)(dg < 0 ? -dg : dg);
                layer_twirl[idx].b = (uint8_t)(db < 0 ? -db : db);
            }
        }
    }
    
    // Step 4: Bilinear Warp & Waveform modulation
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            double warp_x = (double)layer1[idx].r / 255.0 * 2.0;
            double warp_y = (double)layer1[(x % width) * width + (y % height)].g / 255.0 * 2.0;
            
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
            
            warped_r[idx] = 127.5 + 127.5 * tsfi_texgen_sin(r_val * 0.15);
            warped_g[idx] = 127.5 + 127.5 * tsfi_texgen_sin(g_val * 0.15);
            warped_b[idx] = 127.5 + 127.5 * tsfi_texgen_sin(b_val * 0.15);
        }
    }
    
    // Step 5: Normal calculations, Phong highlights & Output packing
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int x_prev = (x - 1 + width) % width;
            int x_next = (x + 1) % width;
            int y_prev = (y - 1 + height) % height;
            int y_next = (y + 1) % height;
            
            int idx = y * width + x;
            double h_center = (warped_r[idx] + warped_g[idx] + warped_b[idx]) / 3.0;
            
            double h_x_next = (warped_r[y*width + x_next] + warped_g[y*width + x_next] + warped_b[y*width + x_next]) / 3.0;
            double h_x_prev = (warped_r[y*width + x_prev] + warped_g[y*width + x_prev] + warped_b[y*width + x_prev]) / 3.0;
            double h_y_next = (warped_r[y_next*width + x] + warped_g[y_next*width + x] + warped_b[y_next*width + x]) / 3.0;
            double h_y_prev = (warped_r[y_prev*width + x] + warped_g[y_prev*width + x] + warped_b[y_prev*width + x]) / 3.0;
            
            double dh_dx = (h_x_next - h_x_prev) / 2.0;
            double dh_dy = (h_y_next - h_y_prev) / 2.0;
            
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
            
            double rz = 2.0 * diffuse * nz - lz;
            double spec = rz;
            if (spec < 0.0) spec = 0.0;
            
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            spec = spec * spec;
            
            double combined_light = diffuse * 0.65 + spec * 0.35;
            if (combined_light > 1.0) combined_light = 1.0;
            
            int out_idx = idx * 3;
            if (mode % 2 == 0) {
                output_rgb[out_idx]   = (uint8_t)(((nx + 1.0) * 127.5) * combined_light);
                output_rgb[out_idx+1] = (uint8_t)(((ny + 1.0) * 127.5) * combined_light);
                output_rgb[out_idx+2] = (uint8_t)(((nz + 1.0) * 127.5) * combined_light);
            } else {
                double intensity = h_center / 255.0 * combined_light;
                if (intensity < 0.5) {
                    double t = intensity * 2.0;
                    output_rgb[out_idx]   = (uint8_t)(2.0 + t * 12.0);
                    output_rgb[out_idx+1] = (uint8_t)(8.0 + t * 157.0);
                    output_rgb[out_idx+2] = (uint8_t)(19.0 + t * 214.0);
                } else {
                    double t = (intensity - 0.5) * 2.0;
                    output_rgb[out_idx]   = (uint8_t)(14.0 + t * 241.0);
                    output_rgb[out_idx+1] = (uint8_t)(165.0 + t * 90.0);
                    output_rgb[out_idx+2] = (uint8_t)(233.0 + t * 22.0);
                }
            }
        }
    }
    
    // Free temporary memory buffers
    free(layer0);
    free(layer1);
    free(layer_twirl);
    free(warped_r);
    free(warped_g);
    free(warped_b);
}
