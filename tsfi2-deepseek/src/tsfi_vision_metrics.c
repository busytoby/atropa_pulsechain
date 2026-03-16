#include "tsfi_vision_internal.h"

bool tsfi_vision_verify_integrity(const TSFiResonanceAnalysis *a) {
    if (!a) return false;
    // Catch signal loss: total darkness or near-zero coverage or total lack of shape fit
    if (a->avg_intensity < 0.01f || a->coverage < 0.01f || a->target_correlation < 0.01f) {
        LOG("\n[!] CRITICAL REGRESSION: Visual Signal Lost (Int: %.2f, Cov: %.2f, Corr: %.2f)\n", a->avg_intensity, a->coverage, a->target_correlation);
        return false;
    }
    return true;
}
__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_similarity(const uint32_t *a, const uint32_t *b, int width, int height) {
    if (!a || !b || width < 16) return 0;
    __m512 v_diff_sum = _mm512_setzero_ps(), v_inv255 = _mm512_set1_ps(1.0f / 255.0f);
    int total = width * height;
    for (size_t i = 0; i < (size_t)total; i += 16) {
        __m512 v_la = get_lum_avx512(safe_load_si512(a, (int)i, total), v_inv255);
        __m512 v_lb = get_lum_avx512(safe_load_si512(b, (int)i, total), v_inv255);
        __m512 v_diff = _mm512_sub_ps(v_la, v_lb);
        v_diff_sum = _mm512_add_ps(v_diff_sum, _mm512_mul_ps(v_diff, v_diff));
    }
    float mse = _mm512_reduce_add_ps(v_diff_sum) / (float)total;
    float score = 1.0f - tsfi_sqrtf(mse);
    return score < 0 ? 0 : score;
}

__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_similarity_ab4h(const uint16_t *a, const uint16_t *b, int width, int height) {
    if (!a || !b || width < 16) return 0;
    __m512 v_diff_sum = _mm512_setzero_ps();
    int total = width * height;
    for (size_t i = 0; i < (size_t)total; i += 16) {
        // Extract G channel for luminance (simplified)
        __m512 v_la = _mm512_cvtph_ps(_mm256_set_epi16(
            a[(i+15)*4+1], a[(i+14)*4+1], a[(i+13)*4+1], a[(i+12)*4+1],
            a[(i+11)*4+1], a[(i+10)*4+1], a[(i+9)*4+1],  a[(i+8)*4+1],
            a[(i+7)*4+1],  a[(i+6)*4+1],  a[(i+5)*4+1],  a[(i+4)*4+1],
            a[(i+3)*4+1],  a[(i+2)*4+1],  a[(i+1)*4+1],  a[(i+0)*4+1]
        ));
        __m512 v_lb = _mm512_cvtph_ps(_mm256_set_epi16(
            b[(i+15)*4+1], b[(i+14)*4+1], b[(i+13)*4+1], b[(i+12)*4+1],
            b[(i+11)*4+1], b[(i+10)*4+1], b[(i+9)*4+1],  b[(i+8)*4+1],
            b[(i+7)*4+1],  b[(i+6)*4+1],  b[(i+5)*4+1],  b[(i+4)*4+1],
            b[(i+3)*4+1],  b[(i+2)*4+1],  b[(i+1)*4+1],  b[(i+0)*4+1]
        ));
        __m512 v_diff = _mm512_sub_ps(v_la, v_lb);
        v_diff_sum = _mm512_add_ps(v_diff_sum, _mm512_mul_ps(v_diff, v_diff));
    }
    float mse = _mm512_reduce_add_ps(v_diff_sum) / (float)total;
    float score = 1.0f - tsfi_sqrtf(mse);
    return score < 0 ? 0 : score;
}

__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_edge_similarity(const uint32_t *a, const uint32_t *b, int width, int height) {
    if (!a || !b || width < 16 || height < 3) return 0;
    __m512 v_diff_sum = _mm512_setzero_ps(), v_inv255 = _mm512_set1_ps(1.0f / 255.0f);
    int total_processed = 0;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 16; x < width - 16; x += 16) {
            int idx = y * width + x;
            
            // Simplified Sobel Magnitude extraction for A
            __m512 v_a_u = get_lum_avx512(safe_load_si512(a, idx - width, width * height), v_inv255);
            __m512 v_a_d = get_lum_avx512(safe_load_si512(a, idx + width, width * height), v_inv255);
            __m512 v_a_l = get_lum_avx512(safe_load_si512(a, idx - 1, width * height), v_inv255);
            __m512 v_a_r = get_lum_avx512(safe_load_si512(a, idx + 1, width * height), v_inv255);
            __m512 v_grad_a = _mm512_add_ps(_mm512_abs_ps(_mm512_sub_ps(v_a_r, v_a_l)), _mm512_abs_ps(_mm512_sub_ps(v_a_d, v_a_u)));

            // Simplified Sobel Magnitude extraction for B
            __m512 v_b_u = get_lum_avx512(safe_load_si512(b, idx - width, width * height), v_inv255);
            __m512 v_b_d = get_lum_avx512(safe_load_si512(b, idx + width, width * height), v_inv255);
            __m512 v_b_l = get_lum_avx512(safe_load_si512(b, idx - 1, width * height), v_inv255);
            __m512 v_b_r = get_lum_avx512(safe_load_si512(b, idx + 1, width * height), v_inv255);
            __m512 v_grad_b = _mm512_add_ps(_mm512_abs_ps(_mm512_sub_ps(v_b_r, v_b_l)), _mm512_abs_ps(_mm512_sub_ps(v_b_d, v_b_u)));

            __m512 v_diff = _mm512_sub_ps(v_grad_a, v_grad_b);
            v_diff_sum = _mm512_add_ps(v_diff_sum, _mm512_mul_ps(v_diff, v_diff));
            total_processed += 16;
        }
    }

    if (total_processed == 0) return 0;
    float mse = _mm512_reduce_add_ps(v_diff_sum) / (float)total_processed;
    float score = 1.0f - tsfi_sqrtf(mse);
    return score < 0 ? 0 : score;
}
float tsfi_vision_calculate_glyph_spacing(const uint32_t *left_glyph, const uint32_t *right_glyph, int width, int height, float target_gap) {
    if (!left_glyph || !right_glyph) return target_gap;

    float left_right_profile[height];
    float right_left_profile[height];
    bool left_has_data[height];
    bool right_has_data[height];
    
    float global_max_right = -10000.0f;
    float global_min_left = 10000.0f;

    for (int y = 0; y < height; y++) {
        left_has_data[y] = false;
        right_has_data[y] = false;
        left_right_profile[y] = -10000.0f;
        right_left_profile[y] = 10000.0f;

        // Find right-most pixel of left_glyph
        for (int x = width - 1; x >= 0; x--) {
            uint8_t a = (left_glyph[y * width + x] >> 24) & 0xFF;
            if (a > 10) {
                left_right_profile[y] = (float)x;
                left_has_data[y] = true;
                if ((float)x > global_max_right) global_max_right = (float)x;
                break;
            }
        }

        // Find left-most pixel of right_glyph
        for (int x = 0; x < width; x++) {
            uint8_t a = (right_glyph[y * width + x] >> 24) & 0xFF;
            if (a > 10) {
                right_left_profile[y] = (float)x;
                right_has_data[y] = true;
                if ((float)x < global_min_left) global_min_left = (float)x;
                break;
            }
        }
    }

    if (global_max_right == -10000.0f || global_min_left == 10000.0f) {
        return target_gap; // Empty glyph(s)
    }

    float max_diag_diff = -10000.0f;
    int overlap_count = 0;
    int search_radius = height / 8; // Vertical radius for diagonal kerning

    for (int y1 = 0; y1 < height; y1++) {
        if (!left_has_data[y1]) continue;
        for (int y2 = y1 - search_radius; y2 <= y1 + search_radius; y2++) {
            if (y2 >= 0 && y2 < height && right_has_data[y2]) {
                float dy = (float)(y1 - y2);
                float penalty = tsfi_sqrtf(dy * dy) * 0.5f; // Diagonal clearance
                float diff = left_right_profile[y1] - right_left_profile[y2] - penalty;
                if (diff > max_diag_diff) max_diag_diff = diff;
                overlap_count++;
            }
        }
    }

    if (overlap_count > 0) {
        return target_gap + max_diag_diff;
    } else {
        // No vertical overlap (e.g., apostrophe and comma), just space bounding boxes
        return target_gap + (global_max_right - global_min_left);
    }
}
