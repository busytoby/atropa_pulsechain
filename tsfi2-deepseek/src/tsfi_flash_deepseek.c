#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_flash_deepseek.h"

bool tsfi_flash_deepseek_forward(
    const float *q,
    const float *k,
    const float *v,
    int seq_len,
    int head_dim,
    float *out,
    tsfi_flash_state_t *flash_out
) {
    if (!q || !k || !v || !out || seq_len <= 0 || head_dim <= 0) return false;

    float scale = 1.0f / sqrtf((float)head_dim);
    int Br = FLASH_TILE_BR;
    int Bc = FLASH_TILE_BC;

    memset(out, 0, (size_t)seq_len * (size_t)head_dim * sizeof(float));

    // Outer loop over Query Tiles Q_i (Br rows)
    for (int i = 0; i < seq_len; i += Br) {
        int i_max = (i + Br < seq_len) ? (i + Br) : seq_len;

        // Inner loop over Key/Value Tiles K_j, V_j (Bc columns)
        for (int j = 0; j < seq_len; j += Bc) {
            int j_max = (j + Bc < seq_len) ? (j + Bc) : seq_len;

            // Compute Tile Matrix S_ij = (Q_i * K_j^T) * scale
            for (int r = i; r < i_max; r++) {
                const float *q_row = q + r * head_dim;
                float row_max = -1e9f;

                // 1. Calculate tile dot products and find row max
                float tile_scores[FLASH_TILE_BC];
                for (int c = j; c < j_max; c++) {
                    const float *k_row = k + c * head_dim;
                    float dot = 0.0f;
                    for (int d = 0; d < head_dim; d++) {
                        dot += q_row[d] * k_row[d];
                    }
                    dot *= scale;
                    tile_scores[c - j] = dot;
                    if (dot > row_max) row_max = dot;
                }

                // 2. Compute online exp and sum
                float tile_sum = 0.0f;
                for (int c = j; c < j_max; c++) {
                    float exp_val = expf(tile_scores[c - j] - row_max);
                    tile_scores[c - j] = exp_val;
                    tile_sum += exp_val;
                }

                // 3. Accumulate weighted values into output tile O_i
                float *out_row = out + r * head_dim;
                for (int c = j; c < j_max; c++) {
                    const float *v_row = v + c * head_dim;
                    float weight = tile_scores[c - j] / (tile_sum + 1e-6f);
                    for (int d = 0; d < head_dim; d++) {
                        out_row[d] += weight * v_row[d];
                    }
                }

                if (flash_out && r == i) {
                    flash_out->online_max = row_max;
                    flash_out->online_sum = tile_sum;
                }
            }
        }
    }

    if (flash_out) {
        flash_out->tile_br = Br;
        flash_out->tile_bc = Bc;
        // Savings: Full N^2 vs Tiled (Br * Bc) SRAM transfers
        size_t full_bytes = (size_t)seq_len * (size_t)seq_len * sizeof(float);
        size_t tiled_bytes = (size_t)seq_len * (size_t)Bc * sizeof(float);
        flash_out->memory_bytes_saved = (full_bytes > tiled_bytes) ? (full_bytes - tiled_bytes) : 0;
    }

    return true;
}
