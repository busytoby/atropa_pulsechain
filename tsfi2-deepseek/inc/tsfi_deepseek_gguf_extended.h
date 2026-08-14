#ifndef TSFI_DEEPSEEK_GGUF_EXTENDED_H
#define TSFI_DEEPSEEK_GGUF_EXTENDED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// ============================================================================
// 1. Transparent Multi-File GGUF Sharding Engine
// ============================================================================

#define TSFI_MAX_GGUF_SHARDS 16
#define TSFI_MAX_SHARD_TENSORS 2048

typedef struct {
    char file_path[256];
    FILE *fp;
    uint64_t file_size;
    uint32_t tensor_count_in_shard;
} tsfi_gguf_shard_t;

typedef struct {
    char tensor_name[128];
    uint32_t shard_index;
    uint64_t offset_in_shard;
    uint64_t size_bytes;
    uint32_t n_dims;
    uint64_t dims[4];
    uint32_t type;
} tsfi_sharded_tensor_entry_t;

typedef struct {
    uint32_t total_shards;
    tsfi_gguf_shard_t shards[TSFI_MAX_GGUF_SHARDS];
    uint32_t total_tensors;
    tsfi_sharded_tensor_entry_t tensors[TSFI_MAX_SHARD_TENSORS];
} tsfi_gguf_shard_registry_t;

bool tsfi_gguf_shards_init(tsfi_gguf_shard_registry_t *reg);
bool tsfi_gguf_shards_add(tsfi_gguf_shard_registry_t *reg, const char *shard_path);
bool tsfi_gguf_shards_read_tensor(
    const tsfi_gguf_shard_registry_t *reg,
    const char *tensor_name,
    void *dest_buf,
    size_t dest_len
);
void tsfi_gguf_shards_close(tsfi_gguf_shard_registry_t *reg);

// ============================================================================
// 2. Dynamic RoPE Scaling & YaRN Metadata Extractor
// ============================================================================

typedef enum {
    TSFI_ROPE_SCALE_NONE = 0,
    TSFI_ROPE_SCALE_LINEAR = 1,
    TSFI_ROPE_SCALE_YARN = 2
} tsfi_rope_scaling_type_t;

typedef struct {
    float freq_base;                // Base frequency (e.g. 10000.0 or 500000.0)
    float freq_scale;               // Frequency scale factor
    tsfi_rope_scaling_type_t type;  // Linear or YaRN
    float yarn_factor;              // Context extension factor (e.g. 4.0 for 32k->128k)
    float yarn_beta_fast;           // YaRN low frequency ramp threshold
    float yarn_beta_slow;           // YaRN high frequency ramp threshold
    uint32_t original_context_len;  // Training context length (e.g. 4096)
} tsfi_rope_parameters_t;

bool tsfi_rope_parameters_init(tsfi_rope_parameters_t *params);
float tsfi_rope_compute_frequency(const tsfi_rope_parameters_t *params, uint32_t dim_index, uint32_t pos);

// ============================================================================
// 3. Sliding Window Attention (SWA) Circular KV Ring Buffer
// ============================================================================

typedef struct {
    uint32_t window_size;           // e.g. 4096 tokens
    uint32_t head_dim;              // Hidden dimension per attention head
    uint32_t num_heads;             // Attention head count
    uint32_t current_head_pos;      // Circular write cursor
    uint32_t total_tokens_written;  // Monotonic token counter
    float *k_buffer;                // Ring buffer: [window_size * num_heads * head_dim]
    float *v_buffer;                // Ring buffer: [window_size * num_heads * head_dim]
} tsfi_swa_ring_buffer_t;

bool tsfi_swa_ring_init(tsfi_swa_ring_buffer_t *ring, uint32_t window, uint32_t heads, uint32_t dim);
void tsfi_swa_ring_free(tsfi_swa_ring_buffer_t *ring);
bool tsfi_swa_ring_append(
    tsfi_swa_ring_buffer_t *ring,
    const float *k_slice,
    const float *v_slice
);
bool tsfi_swa_ring_get_view(
    const tsfi_swa_ring_buffer_t *ring,
    const float **k_ring_out,
    const float **v_ring_out,
    uint32_t *active_tokens_out
);

#endif // TSFI_DEEPSEEK_GGUF_EXTENDED_H
