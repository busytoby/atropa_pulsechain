#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_deepseek_gguf_extended.h"

// ----------------------------------------------------------------------------
// 1. Transparent Multi-File GGUF Sharding Implementation
// ----------------------------------------------------------------------------

bool tsfi_gguf_shards_init(tsfi_gguf_shard_registry_t *reg) {
    if (!reg) return false;
    memset(reg, 0, sizeof(*reg));
    return true;
}

bool tsfi_gguf_shards_add(tsfi_gguf_shard_registry_t *reg, const char *shard_path) {
    if (!reg || !shard_path || reg->total_shards >= TSFI_MAX_GGUF_SHARDS) return false;

    FILE *fp = fopen(shard_path, "rb");
    if (!fp) return false;

    uint32_t idx = reg->total_shards++;
    tsfi_gguf_shard_t *shard = &reg->shards[idx];
    strncpy(shard->file_path, shard_path, sizeof(shard->file_path) - 1);
    shard->fp = fp;

    fseek(fp, 0, SEEK_END);
    shard->file_size = (uint64_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    return true;
}

bool tsfi_gguf_shards_read_tensor(
    const tsfi_gguf_shard_registry_t *reg,
    const char *tensor_name,
    void *dest_buf,
    size_t dest_len
) {
    if (!reg || !tensor_name || !dest_buf || dest_len == 0) return false;

    for (uint32_t i = 0; i < reg->total_tensors; i++) {
        const tsfi_sharded_tensor_entry_t *t = &reg->tensors[i];
        if (strcmp(t->tensor_name, tensor_name) == 0) {
            if (t->shard_index >= reg->total_shards) return false;
            FILE *fp = reg->shards[t->shard_index].fp;
            if (!fp) return false;

            fseek(fp, (long)t->offset_in_shard, SEEK_SET);
            size_t bytes_to_read = (dest_len < t->size_bytes) ? dest_len : (size_t)t->size_bytes;
            return fread(dest_buf, 1, bytes_to_read, fp) == bytes_to_read;
        }
    }
    return false;
}

void tsfi_gguf_shards_close(tsfi_gguf_shard_registry_t *reg) {
    if (!reg) return;
    for (uint32_t i = 0; i < reg->total_shards; i++) {
        if (reg->shards[i].fp) {
            fclose(reg->shards[i].fp);
            reg->shards[i].fp = NULL;
        }
    }
    reg->total_shards = 0;
}

// ----------------------------------------------------------------------------
// 2. Dynamic RoPE Scaling & YaRN Metadata Extractor Implementation
// ----------------------------------------------------------------------------

bool tsfi_rope_parameters_init(tsfi_rope_parameters_t *params) {
    if (!params) return false;
    params->freq_base = 10000.0f;
    params->freq_scale = 1.0f;
    params->type = TSFI_ROPE_SCALE_NONE;
    params->yarn_factor = 1.0f;
    params->yarn_beta_fast = 32.0f;
    params->yarn_beta_slow = 1.0f;
    params->original_context_len = 4096;
    return true;
}

float tsfi_rope_compute_frequency(const tsfi_rope_parameters_t *params, uint32_t dim_index, uint32_t pos) {
    if (!params) return 0.0f;

    float theta = powf(params->freq_base, -2.0f * (float)(dim_index / 2) / (float)128);

    if (params->type == TSFI_ROPE_SCALE_LINEAR) {
        theta /= params->freq_scale;
    } else if (params->type == TSFI_ROPE_SCALE_YARN) {
        // YaRN Interpolation ramp
        float ramp_mix = ((float)dim_index - params->yarn_beta_slow) / (params->yarn_beta_fast - params->yarn_beta_slow + 1e-6f);
        if (ramp_mix < 0.0f) ramp_mix = 0.0f;
        if (ramp_mix > 1.0f) ramp_mix = 1.0f;
        float yarn_theta = theta / params->yarn_factor;
        theta = (1.0f - ramp_mix) * yarn_theta + ramp_mix * theta;
    }

    return (float)pos * theta;
}

// ----------------------------------------------------------------------------
// 3. Sliding Window Attention (SWA) Circular KV Ring Buffer Implementation
// ----------------------------------------------------------------------------

bool tsfi_swa_ring_init(tsfi_swa_ring_buffer_t *ring, uint32_t window, uint32_t heads, uint32_t dim) {
    if (!ring || window == 0 || heads == 0 || dim == 0) return false;
    memset(ring, 0, sizeof(*ring));

    ring->window_size = window;
    ring->num_heads = heads;
    ring->head_dim = dim;

    size_t total_floats = (size_t)window * heads * dim;
    ring->k_buffer = (float *)calloc(total_floats, sizeof(float));
    ring->v_buffer = (float *)calloc(total_floats, sizeof(float));

    if (!ring->k_buffer || !ring->v_buffer) {
        tsfi_swa_ring_free(ring);
        return false;
    }
    return true;
}

void tsfi_swa_ring_free(tsfi_swa_ring_buffer_t *ring) {
    if (!ring) return;
    if (ring->k_buffer) { free(ring->k_buffer); ring->k_buffer = NULL; }
    if (ring->v_buffer) { free(ring->v_buffer); ring->v_buffer = NULL; }
}

bool tsfi_swa_ring_append(
    tsfi_swa_ring_buffer_t *ring,
    const float *k_slice,
    const float *v_slice
) {
    if (!ring || !k_slice || !v_slice || !ring->k_buffer || !ring->v_buffer) return false;

    size_t token_stride = (size_t)ring->num_heads * ring->head_dim;
    size_t offset = (size_t)ring->current_head_pos * token_stride;

    memcpy(ring->k_buffer + offset, k_slice, token_stride * sizeof(float));
    memcpy(ring->v_buffer + offset, v_slice, token_stride * sizeof(float));

    ring->current_head_pos = (ring->current_head_pos + 1) % ring->window_size;
    ring->total_tokens_written++;
    return true;
}

bool tsfi_swa_ring_get_view(
    const tsfi_swa_ring_buffer_t *ring,
    const float **k_ring_out,
    const float **v_ring_out,
    uint32_t *active_tokens_out
) {
    if (!ring || !k_ring_out || !v_ring_out || !active_tokens_out) return false;

    *k_ring_out = ring->k_buffer;
    *v_ring_out = ring->v_buffer;
    *active_tokens_out = (ring->total_tokens_written > ring->window_size) 
                         ? ring->window_size 
                         : ring->total_tokens_written;
    return true;
}
