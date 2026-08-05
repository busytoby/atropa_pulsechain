#include "tsfi_montecarlo.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

bool tsfi_montecarlo_estimate_variance(const float *samples, int count, double threshold, double *variance_out) {
    if (!samples || count <= 1 || !variance_out) {
        return false;
    }
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }
    double mean = sum / count;
    double sq_diff_sum = 0.0;
    for (int i = 0; i < count; i++) {
        double diff = samples[i] - mean;
        sq_diff_sum += diff * diff;
    }
    double variance = sq_diff_sum / (count - 1);
    *variance_out = variance;
    return (variance > threshold);
}

bool tsfi_montecarlo_cross_bilateral_filter(
    const uint32_t *noisy_color,
    const TSFiMCAuxFeatures *features,
    uint32_t *denoised_color,
    int width,
    int height,
    float spatial_sigma,
    float range_sigma
) {
    if (!noisy_color || !features || !denoised_color || width <= 0 || height <= 0) {
        return false;
    }
    if (spatial_sigma <= 0.0f || range_sigma <= 0.0f) {
        return false;
    }

    int radius = (int)ceilf(spatial_sigma * 2.0f);
    float spatial_denom = 2.0f * spatial_sigma * spatial_sigma;
    float range_denom = 2.0f * range_sigma * range_sigma;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            TSFiMCAuxFeatures center_feat = features[idx];
            uint32_t center_pixel = noisy_color[idx];

            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
            float sum_w = 0.0f;

            for (int dy = -radius; dy <= radius; dy++) {
                int ny = y + dy;
                if (ny < 0 || ny >= height) continue;
                for (int dx = -radius; dx <= radius; dx++) {
                    int nx = x + dx;
                    if (nx < 0 || nx >= width) continue;

                    int n_idx = ny * width + nx;
                    TSFiMCAuxFeatures n_feat = features[n_idx];
                    uint32_t n_pixel = noisy_color[n_idx];

                    float nr = (float)((n_pixel >> 16) & 0xFF) / 255.0f;
                    float ng = (float)((n_pixel >> 8) & 0xFF) / 255.0f;
                    float nb = (float)(n_pixel & 0xFF) / 255.0f;

                    // Spatial distance weight
                    float d_spatial_sq = (float)(dx * dx + dy * dy);
                    float w_spatial = expf(-d_spatial_sq / spatial_denom);

                    // Feature distance weight (combines normal difference and depth difference)
                    float d_depth = fabsf(center_feat.depth - n_feat.depth);
                    float dot_normal = center_feat.normal.x * n_feat.normal.x +
                                       center_feat.normal.y * n_feat.normal.y +
                                       center_feat.normal.z * n_feat.normal.z;
                    float d_normal_sq = 2.0f * (1.0f - fmaxf(-1.0f, fminf(1.0f, dot_normal)));
                    
                    float d_range_sq = d_depth * d_depth + d_normal_sq;
                    float w_range = expf(-d_range_sq / range_denom);

                    float w = w_spatial * w_range;
                    sum_r += nr * w;
                    sum_g += ng * w;
                    sum_b += nb * w;
                    sum_w += w;
                }
            }

            if (sum_w > 0.0f) {
                int out_r = (int)fmaxf(0.0f, fminf(255.0f, (sum_r / sum_w) * 255.0f));
                int out_g = (int)fmaxf(0.0f, fminf(255.0f, (sum_g / sum_w) * 255.0f));
                int out_b = (int)fmaxf(0.0f, fminf(255.0f, (sum_b / sum_w) * 255.0f));
                denoised_color[idx] = (center_pixel & 0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;
            } else {
                denoised_color[idx] = center_pixel;
            }
        }
    }
    return true;
}

bool tsfi_montecarlo_non_local_means(
    const float *noisy_input,
    const float *guidance_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius
) {
    if (!noisy_input || !guidance_map || !clean_output || width <= 0 || height <= 0) {
        return false;
    }
    if (filter_strength <= 0.0f || patch_radius < 0 || search_radius < 0) {
        return false;
    }

    float h_sq = filter_strength * filter_strength;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            float sum_val = 0.0f;
            float sum_w = 0.0f;

            for (int sy = -search_radius; sy <= search_radius; sy++) {
                int ny = y + sy;
                if (ny < 0 || ny >= height) continue;
                for (int sx = -search_radius; sx <= search_radius; sx++) {
                    int nx = x + sx;
                    if (nx < 0 || nx >= width) continue;

                    // Compute patch similarity distance using the guidance map
                    float patch_dist_sq = 0.0f;
                    int patch_pixels = 0;

                    for (int py = -patch_radius; py <= patch_radius; py++) {
                        int c_py = y + py;
                        int n_py = ny + py;
                        if (c_py < 0 || c_py >= height || n_py < 0 || n_py >= height) continue;

                        for (int px = -patch_radius; px <= patch_radius; px++) {
                            int c_px = x + px;
                            int n_px = nx + px;
                            if (c_px < 0 || c_px >= width || n_px < 0 || n_px >= width) continue;

                            float diff = guidance_map[c_py * width + c_px] - guidance_map[n_py * width + n_px];
                            patch_dist_sq += diff * diff;
                            patch_pixels++;
                        }
                    }

                    if (patch_pixels > 0) {
                        float mean_dist_sq = patch_dist_sq / patch_pixels;
                        float w = expf(-mean_dist_sq / h_sq);
                        sum_val += noisy_input[ny * width + nx] * w;
                        sum_w += w;
                    }
                }
            }

            if (sum_w > 0.0f) {
                clean_output[idx] = sum_val / sum_w;
            } else {
                clean_output[idx] = noisy_input[idx];
            }
        }
    }
    return true;
}

bool tsfi_montecarlo_emotional_non_local_means(
    const float *noisy_input,
    const float *emotional_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius,
    float empathy_bias
) {
    if (!noisy_input || !emotional_map || !clean_output || width <= 0 || height <= 0) {
        return false;
    }
    if (filter_strength <= 0.0f || patch_radius < 0 || search_radius < 0) {
        return false;
    }

    float h_sq = filter_strength * filter_strength;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            float sum_val = 0.0f;
            float sum_w = 0.0f;

            for (int sy = -search_radius; sy <= search_radius; sy++) {
                int ny = y + sy;
                if (ny < 0 || ny >= height) continue;
                for (int sx = -search_radius; sx <= search_radius; sx++) {
                    int nx = x + sx;
                    if (nx < 0 || nx >= width) continue;

                    // Compute patch similarity distance favoring emotional map features
                    float patch_dist_sq = 0.0f;
                    int patch_pixels = 0;

                    for (int py = -patch_radius; py <= patch_radius; py++) {
                        int c_py = y + py;
                        int n_py = ny + py;
                        if (c_py < 0 || c_py >= height || n_py < 0 || n_py >= height) continue;

                        for (int px = -patch_radius; px <= patch_radius; px++) {
                            int c_px = x + px;
                            int n_px = nx + px;
                            if (c_px < 0 || c_px >= width || n_px < 0 || n_px >= width) continue;

                            float diff_color = noisy_input[c_py * width + c_px] - noisy_input[n_py * width + n_px];
                            float diff_emotion = emotional_map[c_py * width + c_px] - emotional_map[n_py * width + n_px];

                            patch_dist_sq += (1.0f - empathy_bias) * diff_color * diff_color +
                                             empathy_bias * diff_emotion * diff_emotion;
                            patch_pixels++;
                        }
                    }

                    if (patch_pixels > 0) {
                        float mean_dist_sq = patch_dist_sq / patch_pixels;
                        float w = expf(-mean_dist_sq / h_sq);
                        sum_val += noisy_input[ny * width + nx] * w;
                        sum_w += w;
                    }
                }
            }

            if (sum_w > 0.0f) {
                clean_output[idx] = sum_val / sum_w;
            } else {
                clean_output[idx] = noisy_input[idx];
            }
        }
    }
    return true;
}

bool tsfi_montecarlo_guided_path_non_local_means(
    const float *noisy_input,
    const float *emotional_map,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_steps,
    float empathy_bias
) {
    if (!noisy_input || !emotional_map || !clean_output || width <= 0 || height <= 0) {
        return false;
    }
    if (filter_strength <= 0.0f || patch_radius < 0 || search_steps < 0) {
        return false;
    }

    float h_sq = filter_strength * filter_strength;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            float sum_val = 0.0f;
            float sum_w = 0.0f;

            // Compute local emotional map gradient
            float gx = 0.0f;
            float gy = 0.0f;
            if (x > 0 && x < width - 1) {
                gx = emotional_map[idx + 1] - emotional_map[idx - 1];
            }
            if (y > 0 && y < height - 1) {
                gy = emotional_map[idx + width] - emotional_map[idx - width];
            }

            // Normalize gradient
            float mag = sqrtf(gx * gx + gy * gy) + 1e-6f;
            float dir_x = gx / mag;
            float dir_y = gy / mag;

            // Perpendicular direction (tangent along features)
            float tan_x = -dir_y;
            float tan_y = dir_x;

            // We select search paths along both the gradient and tangent vectors
            for (int path = 0; path < 2; path++) {
                float step_x = (path == 0) ? dir_x : tan_x;
                float step_y = (path == 0) ? dir_y : tan_y;

                for (int s = -search_steps; s <= search_steps; s++) {
                    int nx = (int)(x + step_x * s);
                    int ny = (int)(y + step_y * s);

                    if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;

                    float patch_dist_sq = 0.0f;
                    int patch_pixels = 0;

                    for (int py = -patch_radius; py <= patch_radius; py++) {
                        int c_py = y + py;
                        int n_py = ny + py;
                        if (c_py < 0 || c_py >= height || n_py < 0 || n_py >= height) continue;

                        for (int px = -patch_radius; px <= patch_radius; px++) {
                            int c_px = x + px;
                            int n_px = nx + px;
                            if (c_px < 0 || c_px >= width || n_px < 0 || n_px >= width) continue;

                            float diff_color = noisy_input[c_py * width + c_px] - noisy_input[n_py * width + n_px];
                            float diff_emotion = emotional_map[c_py * width + c_px] - emotional_map[n_py * width + n_px];

                            patch_dist_sq += (1.0f - empathy_bias) * diff_color * diff_color +
                                             empathy_bias * diff_emotion * diff_emotion;
                            patch_pixels++;
                        }
                    }

                    if (patch_pixels > 0) {
                        float mean_dist_sq = patch_dist_sq / patch_pixels;
                        float w = expf(-mean_dist_sq / h_sq);
                        sum_val += noisy_input[ny * width + nx] * w;
                        sum_w += w;
                    }
                }
            }

            if (sum_w > 0.0f) {
                clean_output[idx] = sum_val / sum_w;
            } else {
                clean_output[idx] = noisy_input[idx];
            }
        }
    }
    return true;
}

static bool solve_4x4(double A[4][4], double B[4], double x[4]) {
    double mat[4][5];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mat[i][j] = A[i][j];
        }
        mat[i][4] = B[i];
    }

    for (int i = 0; i < 4; i++) {
        int pivot = i;
        for (int r = i + 1; r < 4; r++) {
            if (fabs(mat[r][i]) > fabs(mat[pivot][i])) {
                pivot = r;
            }
        }
        if (pivot != i) {
            for (int c = 0; c < 5; c++) {
                double temp = mat[i][c];
                mat[i][c] = mat[pivot][c];
                mat[pivot][c] = temp;
            }
        }
        if (fabs(mat[i][i]) < 1e-9) {
            return false;
        }
        for (int r = 0; r < 4; r++) {
            if (r != i) {
                double factor = mat[r][i] / mat[i][i];
                for (int c = i; c < 5; c++) {
                    mat[r][c] -= factor * mat[i][c];
                }
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        x[i] = mat[i][4] / mat[i][i];
    }
    return true;
}

bool tsfi_montecarlo_regression_denoise(
    const float *noisy_input,
    const TSFiMCAuxFeatures *features,
    float *clean_output,
    int width,
    int height,
    int window_radius,
    float spatial_sigma,
    float feature_sigma
) {
    if (!noisy_input || !features || !clean_output || width <= 0 || height <= 0) {
        return false;
    }
    if (window_radius < 0 || spatial_sigma <= 0.0f || feature_sigma <= 0.0f) {
        return false;
    }

    float spatial_sig_sq = spatial_sigma * spatial_sigma;
    float feature_sig_sq = feature_sigma * feature_sigma;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            double A[4][4] = {{0}};
            double B[4] = {0};
            
            TSFiMCAuxFeatures c_feat = features[idx];
            float c_albedo = (c_feat.albedo.x + c_feat.albedo.y + c_feat.albedo.z) * 0.3333f;

            for (int wy = -window_radius; wy <= window_radius; wy++) {
                int ny = y + wy;
                if (ny < 0 || ny >= height) continue;
                for (int wx = -window_radius; wx <= window_radius; wx++) {
                    int nx = x + wx;
                    if (nx < 0 || nx >= width) continue;

                    int n_idx = ny * width + nx;
                    TSFiMCAuxFeatures n_feat = features[n_idx];
                    
                    // Compute bilateral weight
                    float s_dist_sq = (float)(wx * wx + wy * wy);
                    float d_depth = c_feat.depth - n_feat.depth;
                    float d_emot = c_feat.emotional_weight - n_feat.emotional_weight;
                    float dot_norm = c_feat.normal.x * n_feat.normal.x +
                                     c_feat.normal.y * n_feat.normal.y +
                                     c_feat.normal.z * n_feat.normal.z;
                    float d_norm_sq = fmaxf(0.0f, 1.0f - dot_norm);
                    
                    float n_albedo = (n_feat.albedo.x + n_feat.albedo.y + n_feat.albedo.z) * 0.3333f;
                    float d_albedo = c_albedo - n_albedo;
                    
                    float f_dist_sq = d_depth * d_depth + d_emot * d_emot + d_norm_sq + d_albedo * d_albedo;
                    
                    double w = exp((double)(-s_dist_sq / spatial_sig_sq - f_dist_sq / feature_sig_sq));

                    // Build regression covariates
                    double v[4];
                    v[0] = 1.0;
                    v[1] = (double)(n_feat.depth - c_feat.depth);
                    v[2] = (double)(n_feat.emotional_weight - c_feat.emotional_weight);
                    v[3] = (double)(n_albedo - c_albedo);

                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            A[i][j] += w * v[i] * v[j];
                        }
                        B[i] += w * v[i] * (double)noisy_input[n_idx];
                    }
                }
            }

            double coeffs[4] = {0};
            if (solve_4x4(A, B, coeffs)) {
                clean_output[idx] = (float)coeffs[0];
            } else {
                clean_output[idx] = noisy_input[idx];
            }
        }
    }
    return true;
}


