#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_staging.h"
#include "lau_memory.h"

int main() {
    printf("=== Testing Staging Buffer SIMD (AVX-512) ===\n");

    size_t w = 128, h = 128;
    StagingBuffer *sb = create_staging_buffer(w, h);
    assert(sb != NULL);

    // 1. Test Clear
    printf("[TEST] Staging Clear (AVX-512)...\n");
    uint32_t color = 0x11223344;
    staging_clear_avx512(sb, color);
    
    uint32_t *px = (uint32_t*)sb->data;
    for(size_t i=0; i < w*h; i++) {
        if(px[i] != color) {
            printf("FAIL: Index %zu has color 0x%08x (Expected 0x%08x)\n", i, px[i], color);
            return 1;
        }
    }
    printf("[PASS] Clear Correct.\n");

    // 2. Test Copy
    printf("[TEST] Staging Copy (AVX-512)...\n");
    uint32_t *src = (uint32_t*)lau_malloc(w * h * 4);
    for(size_t i=0; i < w*h; i++) src[i] = (uint32_t)i;
    
    staging_copy_avx512(sb, src, w * h);
    for(size_t i=0; i < w*h; i++) {
        if(px[i] != (uint32_t)i) {
            printf("FAIL: Copy Index %zu mismatch: 0x%08x (Expected 0x%08x)\n", i, px[i], (uint32_t)i);
            return 1;
        }
    }
    printf("[PASS] Copy Correct.\n");

    // 3. Test Blend Over (Opaque Fill)
    printf("[TEST] Staging Blend Over (AVX-512 Fill)...\n");
    uint32_t fill_color = 0xFFFFFFFF;
    int fx=10, fy=10, fw=50, fh=50;
    staging_blend_over_avx512(sb, fx, fy, fw, fh, fill_color);
    
    for(int j=fy; j < fy+fh; j++) {
        for(int i=fx; i < fx+fw; i++) {
            uint32_t c = px[j * w + i];
            if(c != fill_color) {
                printf("FAIL: Blend at %d,%d is 0x%08x\n", i, j, c);
                return 1;
            }
        }
    }
    printf("[PASS] Blend Over (Fill) Correct.\n");

    destroy_staging_buffer(sb);
    lau_free(src);
    
    printf("=== SIMD TESTS PASSED ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
