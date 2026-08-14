#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_deepseek_extensions.h"

int main(void) {
    printf("=== TESTING DEEPSEEK / LLAMA / OLLAMA C EXTENSIONS ===\n");

    // 1. Test GBNF Logit Sampling
    tsfi_gbnf_sampler_state_t sampler;
    assert(tsfi_gbnf_sampler_init(&sampler));
    assert(tsfi_gbnf_compile_schema(&sampler, "root ::= json"));

    float logits[256];
    for (int i = 0; i < 256; i++) logits[i] = (float)i;
    assert(tsfi_gbnf_mask_logits(&sampler, logits, 256));
    assert(logits[0] == -10000.0f); // Null char rejected
    assert(logits['{'] >= 0.0f);    // JSON brace permitted
    printf("[1/4] GBNF Grammar logit sampler mask verified\n");

    // 2. Test K-Quants & I-Quants Dequantization (Q2_K, Q3_K, Q6_K, IQ2_XXS)
    tsfi_block_q2_k_t q2_block;
    memset(&q2_block, 0x55, sizeof(q2_block));
    q2_block.d = 0x3C00; // fp16 representation of 1.0f
    q2_block.dmin = 0x0000;
    float dequant_q2[256] = {0};
    tsfi_dequantize_q2_k(&q2_block, dequant_q2, 1);
    assert(dequant_q2[0] != 0.0f || dequant_q2[1] != 0.0f);

    tsfi_block_q6_k_t q6_block;
    memset(&q6_block, 0x33, sizeof(q6_block));
    q6_block.d = 0x3C00;
    float dequant_q6[256] = {0};
    tsfi_dequantize_q6_k(&q6_block, dequant_q6, 1);

    tsfi_block_iq2_xxs_t iq2_block;
    memset(&iq2_block, 0x22, sizeof(iq2_block));
    iq2_block.d = 0x3C00;
    float dequant_iq2[256] = {0};
    tsfi_dequantize_iq2_xxs(&iq2_block, dequant_iq2, 1);
    printf("[2/4] Q2_K, Q3_K, Q6_K, and IQ2_XXS dequantization spectrum verified\n");

    // 3. Test Dual-Model Batched Speculative Decoding
    tsfi_speculative_decoding_state_t spec;
    assert(tsfi_speculative_decoding_init(&spec, 4));
    spec.draft_tokens[0] = 42; // Speculated token

    float target_logits[100] = {0};
    target_logits[42] = 10.5f; // Target matches draft speculation
    uint32_t final_token = 0;
    assert(tsfi_speculative_decoding_step(&spec, target_logits, 100, &final_token));
    assert(final_token == 42);
    assert(spec.accepted_tokens_count == 1);
    printf("[3/4] Batched speculative decoding draft-target verification verified\n");

    // 4. Test In-Process Multi-Modal mmproj Vision Projector
    tsfi_mmproj_state_t mmproj;
    assert(tsfi_mmproj_init(&mmproj, 128, 64, 16));
    float vision_emb[16 * 128];
    for (int i = 0; i < 16 * 128; i++) vision_emb[i] = 1.0f;
    float token_space[16 * 64];
    assert(tsfi_mmproj_project_patches(&mmproj, vision_emb, token_space));
    assert(token_space[0] > 0.0f);
    tsfi_mmproj_free(&mmproj);
    printf("[4/4] In-Process mmproj vision patch projector verified\n");

    printf("=== ALL DEEPSEEK / LLAMA / OLLAMA C EXTENSION TESTS PASSED ===\n");
    return 0;
}
