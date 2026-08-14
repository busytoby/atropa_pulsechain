#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_quantized_rag.h"

int main(void) {
    printf("================ TEST SHANGLIN YANG ET AL. (2024) QUANTIZED RAG ================\n");

    float q_sample[16];
    float k_sample[16];
    for (int i = 0; i < 16; i++) {
        q_sample[i] = (float)(i + 1) * 0.25f;
        k_sample[i] = (float)(i + 1) * 0.20f;
    }

    tsfi_quantized_rag_state_t rag;
    bool ok_noise = tsfi_quantized_rag_eval_noise(q_sample, 16, 4, &rag); // Q4_K_M
    assert(ok_noise);
    assert(rag.quantization_type == 4);
    assert(rag.quantization_noise_delta > 0.0f);
    printf("  [PASS] Quantization Noise Delta evaluated (delta_Q = %.4f, lambda_Q = %.4f).\n", rag.quantization_noise_delta, rag.alignment_lambda);

    float align_score = tsfi_quantized_rag_align_score(q_sample, k_sample, 16, rag.alignment_lambda);
    assert(align_score > 0.90f);
    printf("  [PASS] Quantization-Aware Cosine Alignment Score evaluated (S_align = %.4f).\n", align_score);

    bool ok_inject = tsfi_quantized_rag_inject_context(q_sample, 16, k_sample, 0.35f);
    assert(ok_inject);
    printf("  [PASS] RAG Context Vector Injection verified.\n");

    bool ok_calib = tsfi_quantized_rag_layer_entropy_calib(q_sample, 16, 2, 32);
    assert(ok_calib);
    printf("  [PASS] Layer-Wise Quantization Entropy Calibration verified.\n");

    float ppl_val = 0.0f;
    bool ok_ppl = tsfi_quantized_rag_ppl_guard(q_sample, 16, &ppl_val);
    assert(ok_ppl);
    printf("  [PASS] Quantized Perplexity Guard PPL_quant verified (PPL = %.4f <= 15.0).\n", ppl_val);

    float chunk_buf[16];
    size_t chunk_len = 0;
    bool ok_chunk = tsfi_quantized_rag_chunk_overlap(k_sample, 16, 8, 4, chunk_buf, &chunk_len);
    assert(ok_chunk);
    assert(chunk_len == 12);
    printf("  [PASS] Sliding Window Vector Chunk Auditor verified (len = %zu).\n", chunk_len);

    float rerank_score = tsfi_quantized_rag_rerank(q_sample, k_sample, 16, rag.quantization_noise_delta, true);
    assert(rerank_score > 2.0f);
    printf("  [PASS] Multi-Pass Quantization-Aware Re-Ranking verified (Score = %.4f).\n", rerank_score);

    float compressed_buf[16];
    int compressed_dim = 0;
    bool ok_comp = tsfi_quantized_rag_compress_context(k_sample, 16, rag.quantization_noise_delta, compressed_buf, &compressed_dim);
    assert(ok_comp);
    assert(compressed_dim == 8);
    printf("  [PASS] Dynamic Bit-Width Adaptive Context Compression verified (dim = %d).\n", compressed_dim);

    float calib_logit = tsfi_quantized_rag_noise_feedback_calib(10.0f, rag.quantization_noise_delta, 0.12f);
    assert(calib_logit < 10.0f);
    printf("  [PASS] Cross-Layer Quantization Noise Feedback Loop verified (calib = %.4f).\n", calib_logit);

    printf("=================================================================================\n");
    return 0;
}
