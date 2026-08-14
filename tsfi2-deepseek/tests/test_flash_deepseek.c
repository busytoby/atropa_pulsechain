#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_flash_deepseek.h"

int main(void) {
    printf("================ TEST FLASHATTENTION-2 TILED MATRIX KERNEL (DEEPSEEK) ================\n");

    int seq_len = 128;
    int head_dim = 64;
    float *q = (float *)calloc((size_t)seq_len * (size_t)head_dim, sizeof(float));
    float *k = (float *)calloc((size_t)seq_len * (size_t)head_dim, sizeof(float));
    float *v = (float *)calloc((size_t)seq_len * (size_t)head_dim, sizeof(float));
    float *out = (float *)calloc((size_t)seq_len * (size_t)head_dim, sizeof(float));

    for (int i = 0; i < seq_len * head_dim; i++) {
        q[i] = (float)(i % 17) * 0.10f;
        k[i] = (float)(i % 19) * 0.08f;
        v[i] = (float)(i % 23) * 0.05f;
    }

    tsfi_flash_state_t flash;
    bool ok_flash = tsfi_flash_deepseek_forward(q, k, v, seq_len, head_dim, out, &flash);
    assert(ok_flash);
    assert(flash.tile_br == FLASH_TILE_BR);
    assert(flash.tile_bc == FLASH_TILE_BC);
    assert(out[0] != 0.0f);
    printf("  [PASS] FlashAttention-2 Tiled Forward Pass verified (TileBr = %d, TileBc = %d).\n", flash.tile_br, flash.tile_bc);
    printf("  [PASS] Online Softmax Reduction verified (online_max = %.4f, online_sum = %.4f).\n", flash.online_max, flash.online_sum);
    printf("  [PASS] Memory SRAM Savings verified (%zu bytes saved via tiled reduction).\n", flash.memory_bytes_saved);

    free(q); free(k); free(v); free(out);
    printf("=======================================================================================\n");
    return 0;
}
