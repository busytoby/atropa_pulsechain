#ifndef TSFI_FLASH_DEEPSEEK_H
#define TSFI_FLASH_DEEPSEEK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FLASH_TILE_BR 64
#define FLASH_TILE_BC 64

// FlashAttention-2 Tiled Execution State Struct
typedef struct {
    int   tile_br;                // Row block size (64)
    int   tile_bc;                // Column block size (64)
    float online_max;             // Running online softmax max m_i
    float online_sum;             // Running online softmax sum d_i
    size_t memory_bytes_saved;    // SRAM bytes saved via tiling
} tsfi_flash_state_t;

// FlashAttention-2 Tiled Multi-Head Forward Pass over Q, K, V tensors
bool tsfi_flash_deepseek_forward(
    const float *q,             // Query tensor (N x d)
    const float *k,             // Key tensor (N x d)
    const float *v,             // Value tensor (N x d)
    int seq_len,                // Sequence length N
    int head_dim,               // Head dimension d
    float *out,                 // Output tensor (N x d)
    tsfi_flash_state_t *flash_out
);

#endif // TSFI_FLASH_DEEPSEEK_H
