#ifndef TSFI_DEEPSEEK_EXTENSIONS_H
#define TSFI_DEEPSEEK_EXTENSIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// 1. GBNF (GGML BNF) Grammar Logit Sampler Mask Engine
// ============================================================================

#define TSFI_MAX_GBNF_RULES 64
#define TSFI_MAX_VOCAB_SIZE 65536

typedef enum {
    GBNF_RULE_CHAR = 0,
    GBNF_RULE_CHAR_RANGE = 1,
    GBNF_RULE_RULE_REF = 2,
    GBNF_RULE_END = 3
} tsfi_gbnf_rule_type_t;

typedef struct {
    tsfi_gbnf_rule_type_t type;
    uint32_t char_start;
    uint32_t char_end;
    uint32_t subrule_id;
} tsfi_gbnf_element_t;

typedef struct {
    uint32_t rule_id;
    char rule_name[32];
    uint32_t element_count;
    tsfi_gbnf_element_t elements[16];
} tsfi_gbnf_rule_t;

typedef struct {
    uint32_t total_rules;
    uint32_t root_rule_id;
    tsfi_gbnf_rule_t rules[TSFI_MAX_GBNF_RULES];
    uint8_t allowed_token_bitmask[TSFI_MAX_VOCAB_SIZE / 8];
} tsfi_gbnf_sampler_state_t;

bool tsfi_gbnf_sampler_init(tsfi_gbnf_sampler_state_t *sampler);
bool tsfi_gbnf_compile_schema(tsfi_gbnf_sampler_state_t *sampler, const char *gbnf_text);
bool tsfi_gbnf_mask_logits(const tsfi_gbnf_sampler_state_t *sampler, float *logits, uint32_t vocab_size);

// ============================================================================
// 2. Full K-Quants & I-Quants Dequantization Spectrum (Q2_K, Q3_K, Q6_K, IQ2_XXS)
// ============================================================================

// Block Q2_K (256 values per block)
typedef struct {
    uint8_t scales[16]; // 4-bit scales
    uint8_t qs[64];     // 2-bit quants
    uint16_t d;         // fp16 scale
    uint16_t dmin;      // fp16 min scale
} tsfi_block_q2_k_t;

// Block Q3_K (256 values per block)
typedef struct {
    uint8_t hmask[32];  // high bits
    uint8_t qs[64];     // low bits
    uint8_t scales[12]; // scales
    uint16_t d;         // fp16 scale
} tsfi_block_q3_k_t;

// Block Q6_K (256 values per block)
typedef struct {
    uint8_t ql[128];    // 4-bit quants
    uint8_t qh[64];     // 2-bit high quants
    int8_t scales[16];  // 8-bit scales
    uint16_t d;         // fp16 scale
} tsfi_block_q6_k_t;

// Block IQ2_XXS (256 values per block)
typedef struct {
    uint16_t d;         // fp16 scale
    uint8_t qs[64];     // importance index & quant bits
} tsfi_block_iq2_xxs_t;

void tsfi_dequantize_q2_k(const tsfi_block_q2_k_t *blocks, float *out, size_t count);
void tsfi_dequantize_q3_k(const tsfi_block_q3_k_t *blocks, float *out, size_t count);
void tsfi_dequantize_q6_k(const tsfi_block_q6_k_t *blocks, float *out, size_t count);
void tsfi_dequantize_iq2_xxs(const tsfi_block_iq2_xxs_t *blocks, float *out, size_t count);

// ============================================================================
// 3. Batched Speculative Decoding (Dual Draft / Target Model Engine)
// ============================================================================

typedef struct {
    uint32_t draft_speculative_window;
    uint32_t draft_tokens[16];
    float draft_probs[16];
    uint32_t accepted_tokens_count;
    float speculative_acceptance_rate;
} tsfi_speculative_decoding_state_t;

bool tsfi_speculative_decoding_init(tsfi_speculative_decoding_state_t *spec, uint32_t window);
bool tsfi_speculative_decoding_step(
    tsfi_speculative_decoding_state_t *spec,
    const float *target_logits,
    uint32_t target_vocab_size,
    uint32_t *final_token_out
);

// ============================================================================
// 4. In-Process Multi-Modal mmproj Vision Projector
// ============================================================================

typedef struct {
    uint32_t vision_hidden_dim;
    uint32_t text_hidden_dim;
    uint32_t patch_count;
    float *projection_matrix;
} tsfi_mmproj_state_t;

bool tsfi_mmproj_init(tsfi_mmproj_state_t *mmproj, uint32_t vision_dim, uint32_t text_dim, uint32_t patches);
void tsfi_mmproj_free(tsfi_mmproj_state_t *mmproj);
bool tsfi_mmproj_project_patches(
    const tsfi_mmproj_state_t *mmproj,
    const float *vision_embeddings,
    float *token_space_out
);

#endif // TSFI_DEEPSEEK_EXTENSIONS_H
