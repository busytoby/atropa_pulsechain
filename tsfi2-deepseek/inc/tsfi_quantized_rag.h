#ifndef TSFI_QUANTIZED_RAG_H
#define TSFI_QUANTIZED_RAG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Shanglin Yang et al. (2024) Quantized RAG Context State Struct
typedef struct {
    uint32_t quantization_type;      // GGUF Q4_K_M (4), Q8_0 (8), or FP16 (16)
    float    quantization_noise_delta;// \delta_Q: Relative quantization noise
    float    alignment_lambda;        // \lambda_Q: Alignment scaling parameter (default = 0.05)
    float    retrieval_similarity;   // Quantization-aware cosine alignment score
} tsfi_quantized_rag_state_t;

// Evaluates Quantization Noise Delta \delta_Q over activation vector x
bool tsfi_quantized_rag_eval_noise(
    const float *x,
    int dim,
    uint32_t quant_type,
    tsfi_quantized_rag_state_t *rag_out
);

// Quantization-Aware Cosine Context Alignment Score S_align(q, k)
float tsfi_quantized_rag_align_score(
    const float *q,
    const float *k,
    int dim,
    float lambda_q
);

// Injects RAG context vector embedding into activation state x
bool tsfi_quantized_rag_inject_context(
    float *x,
    int dim,
    const float *rag_context_emb,
    float injection_alpha
);

// Layer-Wise Quantization Entropy Calibration: Recalibrates logit entropy across layers
bool tsfi_quantized_rag_layer_entropy_calib(
    float *x,
    int dim,
    int current_layer,
    int total_layers
);

// Quantized Perplexity Guard PPL_quant: Evaluates running perplexity against Q4 threshold
bool tsfi_quantized_rag_ppl_guard(
    const float *cand_logits,
    int count,
    float *running_ppl_out
);

// Cross-Document Sliding Window Subword Chunk Auditor (Yang et al., 2024)
bool tsfi_quantized_rag_chunk_overlap(
    const float *doc_emb,
    size_t doc_len,
    size_t stride,
    size_t overlap,
    float *chunk_out,
    size_t *chunk_len_out
);

// Multi-Pass Quantization-Aware Re-Ranking over retrieved context documents
float tsfi_quantized_rag_rerank(
    const float *query_emb,
    const float *doc_emb,
    int dim,
    float delta_q,
    bool is_syntax_aligned
);

// Dynamic Bit-Width Adaptive Context Compression (Yang et al., 2024)
bool tsfi_quantized_rag_compress_context(
    const float *rag_context_emb,
    int dim,
    float delta_q,
    float *compressed_out,
    int *compressed_dim_out
);

// Cross-Layer Quantization Noise Feedback Loop Correction
float tsfi_quantized_rag_noise_feedback_calib(
    float raw_logit,
    float delta_q,
    float coupling_eta
);

// Christian Lang et al. (September 2024) Linguistic Domain Knowledge Base Extension RAG Evaluator Struct
typedef struct {
    float linguistic_domain_coverage; // Knowledge base coverage ratio
    float semantic_recall_score;      // Linguistic term precision/recall metric
    float qa_accuracy_f1;             // Automatic question answering evaluation F1 score
    float domain_extension_gain;     // KB extension gain over base LLM
    bool  kb_extension_active;        // True if RAG extended linguistic context was injected
} tsfi_linguistic_rag_state_t;

bool tsfi_linguistic_rag_extend_kb(
    const char *prompt,
    float *activation_x,
    int dim,
    tsfi_linguistic_rag_state_t *ling_out
);

bool tsfi_linguistic_rag_eval_qa_performance(
    const char *prompt,
    const float *activation_x,
    int dim,
    tsfi_linguistic_rag_state_t *ling_out
);

#endif // TSFI_QUANTIZED_RAG_H
