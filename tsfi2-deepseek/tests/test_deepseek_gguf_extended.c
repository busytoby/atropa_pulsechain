#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_gguf_extended.h"

int main(void) {
    printf("=== TESTING EXTENDED GGUF & SWA RUNTIME IN C ===\n");

    // 1. Test Sharded GGUF Registry
    tsfi_gguf_shard_registry_t shards;
    assert(tsfi_gguf_shards_init(&shards));
    printf("[1/3] Sharded GGUF registry initialized\n");

    // 2. Test Dynamic RoPE Frequency Calculations with YaRN Context Extension
    tsfi_rope_parameters_t rope;
    assert(tsfi_rope_parameters_init(&rope));
    rope.type = TSFI_ROPE_SCALE_YARN;
    rope.yarn_factor = 4.0f; // 4x context extension

    float f0 = tsfi_rope_compute_frequency(&rope, 0, 100);
    float f1 = tsfi_rope_compute_frequency(&rope, 64, 100);
    assert(f0 > 0.0f && f1 > 0.0f);
    printf("[2/3] Dynamic RoPE & YaRN frequency computation verified (dim 0: %.4f, dim 64: %.4f)\n", f0, f1);

    // 3. Test Sliding Window Attention Circular KV Ring Buffer
    tsfi_swa_ring_buffer_t ring;
    assert(tsfi_swa_ring_init(&ring, 4, 2, 8)); // 4 tokens capacity, 2 heads, 8 dim

    float k_dummy[16];
    float v_dummy[16];
    for (int i = 0; i < 16; i++) { k_dummy[i] = 1.0f; v_dummy[i] = 2.0f; }

    // Write 6 tokens into a 4-token window (verifying wrap-around)
    for (int t = 0; t < 6; t++) {
        assert(tsfi_swa_ring_append(&ring, k_dummy, v_dummy));
    }
    assert(ring.total_tokens_written == 6);
    assert(ring.current_head_pos == 2); // 6 % 4 = 2

    const float *k_view = NULL;
    const float *v_view = NULL;
    uint32_t active_tokens = 0;
    assert(tsfi_swa_ring_get_view(&ring, &k_view, &v_view, &active_tokens));
    assert(active_tokens == 4); // Clamped to window_size
    tsfi_swa_ring_free(&ring);
    printf("[3/3] Sliding Window Attention circular KV buffer wrap-around verified\n");

    printf("=== ALL EXTENDED GGUF & SWA RUNTIME TESTS PASSED ===\n");
    return 0;
}
