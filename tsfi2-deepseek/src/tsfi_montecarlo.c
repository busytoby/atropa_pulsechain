#define _POSIX_C_SOURCE 200809L
#include "tsfi_montecarlo.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

                    // Feature distance weight (combines Markov transition differences for depth and normal vectors)
                    float c_depth_trans = center_feat.depth - ((idx > 0) ? features[idx - 1].depth : center_feat.depth);
                    float n_depth_trans = n_feat.depth - ((n_idx > 0) ? features[n_idx - 1].depth : n_feat.depth);
                    float d_depth = c_depth_trans - n_depth_trans;

                    float c_norm_x_trans = center_feat.normal.x - ((idx > 0) ? features[idx - 1].normal.x : center_feat.normal.x);
                    float n_norm_x_trans = n_feat.normal.x - ((n_idx > 0) ? features[n_idx - 1].normal.x : n_feat.normal.x);
                    float c_norm_y_trans = center_feat.normal.y - ((idx > 0) ? features[idx - 1].normal.y : center_feat.normal.y);
                    float n_norm_y_trans = n_feat.normal.y - ((n_idx > 0) ? features[n_idx - 1].normal.y : n_feat.normal.y);
                    float c_norm_z_trans = center_feat.normal.z - ((idx > 0) ? features[idx - 1].normal.z : center_feat.normal.z);
                    float n_norm_z_trans = n_feat.normal.z - ((n_idx > 0) ? features[n_idx - 1].normal.z : n_feat.normal.z);

                    float diff_norm_x = c_norm_x_trans - n_norm_x_trans;
                    float diff_norm_y = c_norm_y_trans - n_norm_y_trans;
                    float diff_norm_z = c_norm_z_trans - n_norm_z_trans;
                    float d_normal_sq = diff_norm_x * diff_norm_x + diff_norm_y * diff_norm_y + diff_norm_z * diff_norm_z;
                    
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

                            // Compare transition difference states (first derivatives) along the Markov steps
                            float c_color_trans = noisy_input[c_py * width + c_px] - ((c_px > 0) ? noisy_input[c_py * width + c_px - 1] : noisy_input[c_py * width + c_px]);
                            float n_color_trans = noisy_input[n_py * width + n_px] - ((n_px > 0) ? noisy_input[n_py * width + n_px - 1] : noisy_input[n_py * width + n_px]);
                            float diff_color = c_color_trans - n_color_trans;

                            float c_emot_trans = emotional_map[c_py * width + c_px] - ((c_px > 0) ? emotional_map[c_py * width + c_px - 1] : emotional_map[c_py * width + c_px]);
                            float n_emot_trans = emotional_map[n_py * width + n_px] - ((n_px > 0) ? emotional_map[n_py * width + n_px - 1] : emotional_map[n_py * width + n_px]);
                            float diff_emotion = c_emot_trans - n_emot_trans;

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

                            // Compare transition difference states (first derivatives) along the Markov steps
                            float c_color_trans = noisy_input[c_py * width + c_px] - ((c_px > 0) ? noisy_input[c_py * width + c_px - 1] : noisy_input[c_py * width + c_px]);
                            float n_color_trans = noisy_input[n_py * width + n_px] - ((n_px > 0) ? noisy_input[n_py * width + n_px - 1] : noisy_input[n_py * width + n_px]);
                            float diff_color = c_color_trans - n_color_trans;

                            float c_emot_trans = emotional_map[c_py * width + c_px] - ((c_px > 0) ? emotional_map[c_py * width + c_px - 1] : emotional_map[c_py * width + c_px]);
                            float n_emot_trans = emotional_map[n_py * width + n_px] - ((n_px > 0) ? emotional_map[n_py * width + n_px - 1] : emotional_map[n_py * width + n_px]);
                            float diff_emotion = c_emot_trans - n_emot_trans;

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

bool tsfi_montecarlo_aposteriori_error_estimate(
    const float *samples,
    int count,
    double target_error,
    double *error_out
) {
    if (!samples || count <= 1 || !error_out) {
        return false;
    }
    // We treat the sequential samples as states in a Markov chain.
    // Rather than accumulating independent noise, we compute the transition variance
    // representing the mutation step deviation of the Markov structure.
    double transition_sum = 0.0;
    for (int i = 1; i < count; i++) {
        double diff = samples[i] - samples[i-1];
        transition_sum += diff * diff;
    }
    double transition_variance = transition_sum / (count - 1);

    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }
    double mean = sum / count;
    double error = sqrt(transition_variance) / (mean + 1e-5);
    *error_out = error;
    return (error > target_error);
}

int tsfi_montecarlo_apriori_sample_count(
    const TSFiMCAuxFeatures *features,
    int x,
    int y,
    int width,
    int height,
    int base_samples
) {
    if (!features || width <= 0 || height <= 0 || x < 0 || x >= width || y < 0 || y >= height) {
        return base_samples;
    }

    float grad_depth_x = 0.0f;
    float grad_depth_y = 0.0f;
    int idx = y * width + x;

    if (x > 0 && x < width - 1) {
        grad_depth_x = fabsf(features[idx + 1].depth - features[idx - 1].depth);
    }
    if (y > 0 && y < height - 1) {
        grad_depth_y = fabsf(features[idx + width].depth - features[idx - width].depth);
    }

    float grad_mag = sqrtf(grad_depth_x * grad_depth_x + grad_depth_y * grad_depth_y);

    if (grad_mag > 1.0f) {
        return base_samples * 2;
    } else if (grad_mag > 0.3f) {
        return (int)((float)base_samples * 1.5f);
    }
    return base_samples;
}

bool tsfi_montecarlo_adaptive_sigma(
    double local_error,
    float base_spatial_sigma,
    float base_range_sigma,
    float *adaptive_spatial,
    float *adaptive_range
) {
    if (!adaptive_spatial || !adaptive_range) {
        return false;
    }

    // Model the response as a continuous Darlington comparator transconductance curve
    // using the tanh activation to simulate the cascaded base-emitter saturation loop.
    double gain = tanh(20.0 * local_error); // High transconductance gain scaling

    *adaptive_spatial = base_spatial_sigma * (float)(0.8 + 0.7 * gain);
    *adaptive_range = base_range_sigma * (float)(1.2 - 0.5 * gain);
    return true;
}

TSFiMCFilterTx tsfi_montecarlo_begin_filter_transaction(TSFiMCFilterState *state) {
    TSFiMCFilterTx tx;
    memset(&tx, 0, sizeof(TSFiMCFilterTx));
    if (state) {
        tx.target = state;
        tx.backup = *state;
        tx.active = true;
    }
    return tx;
}

bool tsfi_montecarlo_commit_filter_transaction(TSFiMCFilterTx *tx, float next_spatial, float next_range) {
    if (!tx || !tx->active || !tx->target) {
        return false;
    }

    // Consistency Check: parameters must be strictly positive and finite
    if (next_spatial <= 0.0f || next_range <= 0.0f || isnan(next_spatial) || isnan(next_range)) {
        // Constraint failed: rollback (Atomicity check)
        tsfi_montecarlo_rollback_filter_transaction(tx);
        return false;
    }

    tx->target->spatial_sigma = next_spatial;
    tx->target->range_sigma = next_range;
    tx->active = false;

    // Durability: Persist the committed parameters to a binary transaction log file
    FILE *f = fopen("mc_filter_transaction.dat.bin", "wb");
    if (f) {
        fwrite(tx->target, sizeof(TSFiMCFilterState), 1, f);
        fclose(f);
    }
    return true;
}

void tsfi_montecarlo_rollback_filter_transaction(TSFiMCFilterTx *tx) {
    if (tx && tx->active && tx->target) {
        *tx->target = tx->backup;
        tx->active = false;
    }
}

bool tsfi_montecarlo_collaborative_block_matching_filter(
    const float *noisy_input,
    float *clean_output,
    int width,
    int height,
    float filter_strength,
    int patch_radius,
    int search_radius
) {
    if (!noisy_input || !clean_output || width <= 0 || height <= 0 || filter_strength <= 0.0f) {
        return false;
    }

    float h_sq = filter_strength * filter_strength;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float total_weight = 0.0f;
            float filtered_pixel_val = 0.0f;

            for (int sy = -search_radius; sy <= search_radius; sy++) {
                int ny = y + sy;
                if (ny < 0 || ny >= height) continue;

                for (int sx = -search_radius; sx <= search_radius; sx++) {
                    int nx = x + sx;
                    if (nx < 0 || nx >= width) continue;

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

                            float diff = noisy_input[c_py * width + c_px] - noisy_input[n_py * width + n_px];
                            patch_dist_sq += diff * diff;
                            patch_pixels++;
                        }
                    }

                    if (patch_pixels > 0) {
                        float dist_normalized = patch_dist_sq / patch_pixels;
                        float w = expf(-dist_normalized / h_sq);
                        total_weight += w;
                        filtered_pixel_val += w * noisy_input[ny * width + nx];
                    }
                }
            }

            if (total_weight > 0.0f) {
                clean_output[y * width + x] = filtered_pixel_val / total_weight;
            } else {
                clean_output[y * width + x] = noisy_input[y * width + x];
            }
        }
    }

    return true;
}

bool tsfi_montecarlo_parse_deepseek_guide(
    const char *deepseek_response,
    float *guide_map_out,
    int width,
    int height
) {
    if (!deepseek_response || !guide_map_out || width <= 0 || height <= 0) {
        return false;
    }

    int total_pixels = width * height;
    int count = 0;
    
    // Copy response to parse tokens
    char *dup = strdup(deepseek_response);
    if (!dup) return false;

    char *token = strtok(dup, " ,\t\n\r");
    while (token && count < total_pixels) {
        float val = (float)atof(token);
        guide_map_out[count++] = val;
        token = strtok(NULL, " ,\t\n\r");
    }
    free(dup);

    // If response was short, pad remainder with the last valid value (or 0.0f)
    float pad_val = (count > 0) ? guide_map_out[count - 1] : 0.0f;
    while (count < total_pixels) {
        guide_map_out[count++] = pad_val;
    }

    return true;
}

bool tsfi_montecarlo_spatiotemporal_bilateral_filter(
    const float *current_noisy,
    const float *prev_clean,
    const float *motion_vectors,
    const TSFiMCAuxFeatures *features,
    float *clean_output,
    int width,
    int height,
    float spatial_sigma,
    float feature_sigma,
    float temporal_sigma
) {
    if (!current_noisy || !prev_clean || !motion_vectors || !features || !clean_output ||
        width <= 0 || height <= 0 || spatial_sigma <= 0.0f || feature_sigma <= 0.0f || temporal_sigma <= 0.0f) {
        return false;
    }

    float spatial_denom = 2.0f * spatial_sigma * spatial_sigma;
    float feature_denom = 2.0f * feature_sigma * feature_sigma;
    float temporal_denom = 2.0f * temporal_sigma * temporal_sigma;
    int radius = (int)(spatial_sigma * 2.0f);
    if (radius < 1) radius = 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            TSFiMCAuxFeatures center_feat = features[idx];

            float total_weight = 0.0f;
            float filtered_val = 0.0f;

            // 1. Spatial & Feature Cross-Bilateral loop
            for (int dy = -radius; dy <= radius; dy++) {
                int ny = y + dy;
                if (ny < 0 || ny >= height) continue;

                for (int dx = -radius; dx <= radius; dx++) {
                    int nx = x + dx;
                    if (nx < 0 || nx >= width) continue;

                    int n_idx = ny * width + nx;
                    TSFiMCAuxFeatures n_feat = features[n_idx];

                    float d_spatial_sq = (float)(dx * dx + dy * dy);
                    float w_spatial = expf(-d_spatial_sq / spatial_denom);

                    float c_depth_trans = center_feat.depth - ((idx > 0) ? features[idx - 1].depth : center_feat.depth);
                    float n_depth_trans = n_feat.depth - ((n_idx > 0) ? features[n_idx - 1].depth : n_feat.depth);
                    float d_depth = c_depth_trans - n_depth_trans;

                    float c_norm_x_trans = center_feat.normal.x - ((idx > 0) ? features[idx - 1].normal.x : center_feat.normal.x);
                    float n_norm_x_trans = n_feat.normal.x - ((n_idx > 0) ? features[n_idx - 1].normal.x : n_feat.normal.x);
                    float c_norm_y_trans = center_feat.normal.y - ((idx > 0) ? features[idx - 1].normal.y : center_feat.normal.y);
                    float n_norm_y_trans = n_feat.normal.y - ((n_idx > 0) ? features[n_idx - 1].normal.y : n_feat.normal.y);
                    float c_norm_z_trans = center_feat.normal.z - ((idx > 0) ? features[idx - 1].normal.z : center_feat.normal.z);
                    float n_norm_z_trans = n_feat.normal.z - ((n_idx > 0) ? features[n_idx - 1].normal.z : n_feat.normal.z);

                    float diff_norm_x = c_norm_x_trans - n_norm_x_trans;
                    float diff_norm_y = c_norm_y_trans - n_norm_y_trans;
                    float diff_norm_z = c_norm_z_trans - n_norm_z_trans;
                    float d_normal_sq = diff_norm_x * diff_norm_x + diff_norm_y * diff_norm_y + diff_norm_z * diff_norm_z;

                    float d_range_sq = d_depth * d_depth + d_normal_sq;
                    float w_range = expf(-d_range_sq / feature_denom);

                    float w = w_spatial * w_range;
                    total_weight += w;
                    filtered_val += w * current_noisy[n_idx];
                }
            }

            // 2. Temporal Reprojection Weight via Motion Vectors
            float mv_x = motion_vectors[idx * 2 + 0];
            float mv_y = motion_vectors[idx * 2 + 1];
            int prev_x = (int)roundf((float)x - mv_x);
            int prev_y = (int)roundf((float)y - mv_y);

            if (prev_x >= 0 && prev_x < width && prev_y >= 0 && prev_y < height) {
                int prev_idx = prev_y * width + prev_x;
                float current_val = (total_weight > 0.0f) ? filtered_val / total_weight : current_noisy[idx];
                
                float d_temp = current_val - prev_clean[prev_idx];
                float w_temp = expf(-(d_temp * d_temp) / temporal_denom);

                // Blend temporal reprojected value with spatial filter
                clean_output[idx] = (1.0f - w_temp) * current_val + w_temp * prev_clean[prev_idx];
            } else {
                clean_output[idx] = (total_weight > 0.0f) ? filtered_val / total_weight : current_noisy[idx];
            }
        }
    }

    return true;
}

bool tsfi_montecarlo_estimate_feature_sigmas(
    const TSFiMCAuxFeatures *features,
    int width,
    int height,
    double *depth_sigma_out,
    double *normal_sigma_out,
    double *albedo_sigma_out
) {
    if (!features || width <= 0 || height <= 0 || !depth_sigma_out || !normal_sigma_out || !albedo_sigma_out) {
        return false;
    }

    int total_pixels = width * height;

    // 1. Calculate means
    double depth_sum = 0.0;
    double norm_x_sum = 0.0, norm_y_sum = 0.0, norm_z_sum = 0.0;
    double alb_x_sum = 0.0, alb_y_sum = 0.0, alb_z_sum = 0.0;

    for (int i = 0; i < total_pixels; i++) {
        depth_sum += features[i].depth;
        norm_x_sum += features[i].normal.x;
        norm_y_sum += features[i].normal.y;
        norm_z_sum += features[i].normal.z;
        alb_x_sum += features[i].albedo.x;
        alb_y_sum += features[i].albedo.y;
        alb_z_sum += features[i].albedo.z;
    }

    double depth_mean = depth_sum / total_pixels;
    double norm_x_mean = norm_x_sum / total_pixels;
    double norm_y_mean = norm_y_sum / total_pixels;
    double norm_z_mean = norm_z_sum / total_pixels;
    double alb_x_mean = alb_x_sum / total_pixels;
    double alb_y_mean = alb_y_sum / total_pixels;
    double alb_z_mean = alb_z_sum / total_pixels;

    // 2. Calculate standard deviations
    double depth_sq_diff_sum = 0.0;
    double norm_sq_diff_sum = 0.0;
    double alb_sq_diff_sum = 0.0;

    for (int i = 0; i < total_pixels; i++) {
        double d_diff = features[i].depth - depth_mean;
        depth_sq_diff_sum += d_diff * d_diff;

        double nx_diff = features[i].normal.x - norm_x_mean;
        double ny_diff = features[i].normal.y - norm_y_mean;
        double nz_diff = features[i].normal.z - norm_z_mean;
        norm_sq_diff_sum += (nx_diff * nx_diff + ny_diff * ny_diff + nz_diff * nz_diff);

        double ax_diff = features[i].albedo.x - alb_x_mean;
        double ay_diff = features[i].albedo.y - alb_y_mean;
        double az_diff = features[i].albedo.z - alb_z_mean;
        alb_sq_diff_sum += (ax_diff * ax_diff + ay_diff * ay_diff + az_diff * az_diff);
    }

    *depth_sigma_out = sqrt(depth_sq_diff_sum / (total_pixels - 1));
    *normal_sigma_out = sqrt(norm_sq_diff_sum / (total_pixels - 1));
    *albedo_sigma_out = sqrt(alb_sq_diff_sum / (total_pixels - 1));

    // Guard against zero standard deviation to prevent division-by-zero
    if (*depth_sigma_out < 1e-5) *depth_sigma_out = 1e-5;
    if (*normal_sigma_out < 1e-5) *normal_sigma_out = 1e-5;
    if (*albedo_sigma_out < 1e-5) *albedo_sigma_out = 1e-5;

    return true;
}


