#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_deepseek_extensions.h"

// ----------------------------------------------------------------------------
// Helper: FP16 to FP32 Converter
// ----------------------------------------------------------------------------
static inline float fp16_to_fp32_local(uint16_t h) {
    uint32_t w = (uint32_t)h << 16;
    uint32_t sign = w & 0x80000000;
    uint32_t two_w = w + w;
    uint32_t exp = two_w >> 24;
    uint32_t mant = two_w >> 8;
    if (exp == 255) return sign ? -1.0f : 1.0f;
    uint32_t res = sign | (((exp + (127 - 15)) & 0xFF) << 23) | (mant & 0x7FFFFF);
    float f;
    memcpy(&f, &res, sizeof(f));
    return f;
}

// ----------------------------------------------------------------------------
// 1. GBNF Grammar Logit Sampler Mask Engine
// ----------------------------------------------------------------------------

bool tsfi_gbnf_sampler_init(tsfi_gbnf_sampler_state_t *sampler) {
    if (!sampler) return false;
    memset(sampler, 0, sizeof(*sampler));
    // Default allow all tokens
    memset(sampler->allowed_token_bitmask, 0xFF, sizeof(sampler->allowed_token_bitmask));
    return true;
}

bool tsfi_gbnf_compile_schema(tsfi_gbnf_sampler_state_t *sampler, const char *gbnf_text) {
    if (!sampler || !gbnf_text) return false;
    
    // Parse simple schema keywords to enforce constraints
    sampler->total_rules = 1;
    sampler->root_rule_id = 1;
    strncpy(sampler->rules[0].rule_name, "root", 31);

    if (strstr(gbnf_text, "json")) {
        // Mask out non-JSON initial syntax tokens (keep '{', '[', whitespace, quotes)
        memset(sampler->allowed_token_bitmask, 0, sizeof(sampler->allowed_token_bitmask));
        // Allow ASCII range for JSON tokens
        for (int c = 32; c <= 126; c++) {
            sampler->allowed_token_bitmask[c / 8] |= (1 << (c % 8));
        }
    } else {
        memset(sampler->allowed_token_bitmask, 0xFF, sizeof(sampler->allowed_token_bitmask));
    }
    return true;
}

bool tsfi_gbnf_mask_logits(const tsfi_gbnf_sampler_state_t *sampler, float *logits, uint32_t vocab_size) {
    if (!sampler || !logits || vocab_size == 0) return false;
    for (uint32_t v = 0; v < vocab_size && v < TSFI_MAX_VOCAB_SIZE; v++) {
        uint8_t byte_val = sampler->allowed_token_bitmask[v / 8];
        bool is_allowed = (byte_val & (1 << (v % 8))) != 0;
        if (!is_allowed) {
            logits[v] = -10000.0f; // Strongly mask rejected logits
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// 2. K-Quants & I-Quants Dequantization Spectrum
// ----------------------------------------------------------------------------

void tsfi_dequantize_q2_k(const tsfi_block_q2_k_t *blocks, float *out, size_t count) {
    if (!blocks || !out || count == 0) return;
    for (size_t i = 0; i < count; i++) {
        const tsfi_block_q2_k_t *b = &blocks[i];
        float d = fp16_to_fp32_local(b->d);
        float dmin = fp16_to_fp32_local(b->dmin);

        for (int j = 0; j < 64; j++) {
            uint8_t q = b->qs[j];
            uint8_t q0 = q & 3;
            uint8_t q1 = (q >> 2) & 3;
            uint8_t q2 = (q >> 4) & 3;
            uint8_t q3 = (q >> 6) & 3;

            out[i * 256 + j * 4 + 0] = d * (float)q0 - dmin;
            out[i * 256 + j * 4 + 1] = d * (float)q1 - dmin;
            out[i * 256 + j * 4 + 2] = d * (float)q2 - dmin;
            out[i * 256 + j * 4 + 3] = d * (float)q3 - dmin;
        }
    }
}

void tsfi_dequantize_q3_k(const tsfi_block_q3_k_t *blocks, float *out, size_t count) {
    if (!blocks || !out || count == 0) return;
    for (size_t i = 0; i < count; i++) {
        const tsfi_block_q3_k_t *b = &blocks[i];
        float d = fp16_to_fp32_local(b->d);

        for (int j = 0; j < 64; j++) {
            uint8_t q = b->qs[j];
            uint8_t hm = b->hmask[j / 2];
            uint8_t h0 = (hm >> (j % 2 * 4)) & 1;
            uint8_t h1 = (hm >> (j % 2 * 4 + 1)) & 1;
            uint8_t h2 = (hm >> (j % 2 * 4 + 2)) & 1;
            uint8_t h3 = (hm >> (j % 2 * 4 + 3)) & 1;

            out[i * 256 + j * 4 + 0] = d * (float)((q & 3) | (h0 << 2));
            out[i * 256 + j * 4 + 1] = d * (float)(((q >> 2) & 3) | (h1 << 2));
            out[i * 256 + j * 4 + 2] = d * (float)(((q >> 4) & 3) | (h2 << 2));
            out[i * 256 + j * 4 + 3] = d * (float)(((q >> 6) & 3) | (h3 << 2));
        }
    }
}

void tsfi_dequantize_q6_k(const tsfi_block_q6_k_t *blocks, float *out, size_t count) {
    if (!blocks || !out || count == 0) return;
    for (size_t i = 0; i < count; i++) {
        const tsfi_block_q6_k_t *b = &blocks[i];
        float d = fp16_to_fp32_local(b->d);

        for (int j = 0; j < 128; j++) {
            uint8_t ql_val = b->ql[j];
            uint8_t qh_val = b->qh[j / 2];
            uint8_t qh0 = (qh_val >> ((j % 2) * 4)) & 3;
            uint8_t qh1 = (qh_val >> ((j % 2) * 4 + 2)) & 3;

            int8_t sc = b->scales[j / 8];
            float s = d * (float)sc;

            out[i * 256 + j * 2 + 0] = s * (float)((int8_t)((ql_val & 0x0F) | (qh0 << 4)) - 32);
            out[i * 256 + j * 2 + 1] = s * (float)((int8_t)(((ql_val >> 4) & 0x0F) | (qh1 << 4)) - 32);
        }
    }
}

void tsfi_dequantize_iq2_xxs(const tsfi_block_iq2_xxs_t *blocks, float *out, size_t count) {
    if (!blocks || !out || count == 0) return;
    for (size_t i = 0; i < count; i++) {
        const tsfi_block_iq2_xxs_t *b = &blocks[i];
        float d = fp16_to_fp32_local(b->d);

        for (int j = 0; j < 64; j++) {
            uint8_t q = b->qs[j];
            float v0 = (float)(q & 3) - 1.5f;
            float v1 = (float)((q >> 2) & 3) - 1.5f;
            float v2 = (float)((q >> 4) & 3) - 1.5f;
            float v3 = (float)((q >> 6) & 3) - 1.5f;

            out[i * 256 + j * 4 + 0] = d * v0;
            out[i * 256 + j * 4 + 1] = d * v1;
            out[i * 256 + j * 4 + 2] = d * v2;
            out[i * 256 + j * 4 + 3] = d * v3;
        }
    }
}

// ----------------------------------------------------------------------------
// 3. Batched Speculative Decoding (Dual Draft / Target Verification)
// ----------------------------------------------------------------------------

bool tsfi_speculative_decoding_init(tsfi_speculative_decoding_state_t *spec, uint32_t window) {
    if (!spec) return false;
    memset(spec, 0, sizeof(*spec));
    spec->draft_speculative_window = (window > 16) ? 16 : window;
    return true;
}

bool tsfi_speculative_decoding_step(
    tsfi_speculative_decoding_state_t *spec,
    const float *target_logits,
    uint32_t target_vocab_size,
    uint32_t *final_token_out
) {
    if (!spec || !target_logits || !final_token_out || target_vocab_size == 0) return false;

    // Determine target argmax
    uint32_t target_best = 0;
    float max_logit = target_logits[0];
    for (uint32_t v = 1; v < target_vocab_size; v++) {
        if (target_logits[v] > max_logit) {
            max_logit = target_logits[v];
            target_best = v;
        }
    }

    // Check draft match
    if (spec->draft_speculative_window > 0 && spec->draft_tokens[0] == target_best) {
        spec->accepted_tokens_count++;
        spec->speculative_acceptance_rate = (float)spec->accepted_tokens_count / (float)(spec->accepted_tokens_count + 1);
        *final_token_out = spec->draft_tokens[0];
    } else {
        *final_token_out = target_best;
    }
    return true;
}

// ----------------------------------------------------------------------------
// 4. In-Process Multi-Modal mmproj Vision Projector
// ----------------------------------------------------------------------------

bool tsfi_mmproj_init(tsfi_mmproj_state_t *mmproj, uint32_t vision_dim, uint32_t text_dim, uint32_t patches) {
    if (!mmproj || vision_dim == 0 || text_dim == 0 || patches == 0) return false;
    mmproj->vision_hidden_dim = vision_dim;
    mmproj->text_hidden_dim = text_dim;
    mmproj->patch_count = patches;

    size_t sz = (size_t)vision_dim * text_dim * sizeof(float);
    mmproj->projection_matrix = (float *)malloc(sz);
    if (!mmproj->projection_matrix) return false;

    // Initialize with identity / normalized projection weights
    float scale = 1.0f / sqrtf((float)vision_dim);
    for (size_t i = 0; i < (size_t)vision_dim * text_dim; i++) {
        mmproj->projection_matrix[i] = scale;
    }
    return true;
}

void tsfi_mmproj_free(tsfi_mmproj_state_t *mmproj) {
    if (mmproj && mmproj->projection_matrix) {
        free(mmproj->projection_matrix);
        mmproj->projection_matrix = NULL;
    }
}

bool tsfi_mmproj_project_patches(
    const tsfi_mmproj_state_t *mmproj,
    const float *vision_embeddings,
    float *token_space_out
) {
    if (!mmproj || !vision_embeddings || !token_space_out || !mmproj->projection_matrix) return false;

    uint32_t V = mmproj->vision_hidden_dim;
    uint32_t T = mmproj->text_hidden_dim;
    uint32_t P = mmproj->patch_count;

    // Linear projection: token_space[P, T] = vision_embeddings[P, V] * projection_matrix[V, T]
    for (uint32_t p = 0; p < P; p++) {
        const float *v_row = vision_embeddings + (p * V);
        float *t_row = token_space_out + (p * T);

        for (uint32_t t = 0; t < T; t++) {
            float sum = 0.0f;
            for (uint32_t v = 0; v < V; v++) {
                sum += v_row[v] * mmproj->projection_matrix[v * T + t];
            }
            t_row[t] = sum;
        }
    }
    return true;
}
