#include <immintrin.h>
#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// --- Dual Wave Ray-Marching Kernel ---
// Wave 1 (VGPR/ZMM0-2): Ray Origin (X,Y) and Direction (Z ignored for 2D)
// Wave 2 (SGPR/ZMM16-31): SVDAG Topology Stream (Read-Only)
// Output (VGPR/ZMM0): Pixel Intensity (Energy)

// Simplified Ray-SVDAG Intersector for 2D
// We treat the ZMM registers as a bundle of 16 Rays.
void kernel_raymarch_svdag(void *ctx, ZmmSynapse *syn) {
    (void)syn;
    __m512 *r = (__m512*)ctx;
    
    // Input:
    // r[0] = Ray Origin X (Pixel X normalized)
    // r[1] = Ray Origin Y (Pixel Y normalized)
    // r[4] = Accumulator (Energy/Intensity)
    
    // Bounds Check: -1.0 < X < 1.0 && -1.0 < Y < 1.0
    __m512 neg_one = _mm512_set1_ps(-1.0f);
    __m512 pos_one = _mm512_set1_ps(1.0f);
    
    __mmask16 mask_x = _mm512_cmp_ps_mask(r[0], neg_one, _CMP_GT_OQ) & 
                       _mm512_cmp_ps_mask(r[0], pos_one, _CMP_LT_OQ);
    __mmask16 mask_y = _mm512_cmp_ps_mask(r[1], neg_one, _CMP_GT_OQ) & 
                       _mm512_cmp_ps_mask(r[1], pos_one, _CMP_LT_OQ);
                       
    __mmask16 active_rays = mask_x & mask_y;
    
    // Calculate distance from center (0,0) -> Circle
    __m512 dist_sq = _mm512_add_ps(_mm512_mul_ps(r[0], r[0]), _mm512_mul_ps(r[1], r[1]));
    __m512 inv_dist = _mm512_rsqrt14_ps(dist_sq); // 1/dist
    
    // Masked Accumulation: Energy += 1.0 / Dist (Glowing Core)
    r[4] = _mm512_mask_add_ps(r[4], active_rays, r[4], inv_dist);
}

// Helper: Write PPM
void write_ppm(const char *filename, int w, int h, float *buffer) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        float val = buffer[i];
        if (val > 1.0f) val = 1.0f;
        if (val < 0.0f) val = 0.0f;
        unsigned char c = (unsigned char)(val * 255.0f);
        fputc(c, f); // R
        fputc(c, f); // G
        fputc(c, f); // B
    }
    fclose(f);
    printf("[VISION] Wrote %s\n", filename);
}

void test_symmetric_vrs_verification() {
    printf("\n=== TSFi Symmetric VRS Verification ===\n");
    
    // 1. Create SVDAG
    TSFiHelmholtzSVDAG* dag = tsfi_svdag_create(1024);
    dag->stream_size = 64; // Test subset
    
    // 2. Setup Voxel Intensities
    // [0-15]: High (0.9) -> Should be 1x1 (0x0)
    // [16-31]: Mid (0.5) -> Should be 2x2 (0x5)
    // [32-47]: Low (0.1) -> Should be 4x4 (0xA)
    for (int i = 0; i < 16; i++) dag->intensity_stream[i] = 0.9f;
    for (int i = 16; i < 32; i++) dag->intensity_stream[i] = 0.5f;
    for (int i = 32; i < 48; i++) dag->intensity_stream[i] = 0.1f;

    // 3. Generate VRS Map
    printf("[VRS] Generating Map from Intensity Stream...\n");
    tsfi_svdag_generate_vrs(dag);
    
    // 4. Verify Results
    printf("[VRS] Verifying 1x1 Zone: 0x%02X\n", dag->vrs_map[0]);
    printf("[VRS] Verifying 2x2 Zone: 0x%02X\n", dag->vrs_map[16]);
    printf("[VRS] Verifying 4x4 Zone: 0x%02X\n", dag->vrs_map[32]);

    if (dag->vrs_map[0] == 0x0 && dag->vrs_map[16] == 0x5 && dag->vrs_map[32] == 0xA) {
        printf("[PASS] Symmetric VRS Map Correctly Realized.\n");
    } else {
        printf("[FAIL] VRS Mapping Discrepancy Detected.\n");
        exit(1);
    }

    tsfi_svdag_destroy(dag);
}

int main() {
    printf("[VISION] Starting Dual-Wave SVDAG Render Test...\n");
    
    // 1. Setup SVDAG (The Genome)
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(4096);
    float *stream_f = (float*)dag->intensity_stream;
    stream_f[0] = 1.0f; // Root Density
    
    // 2. Setup Framebuffer
    int W = 256;
    int H = 256;
    float *pixels = (float*)lau_memalign(64, (size_t)W * H * sizeof(float));
    memset(pixels, 0, W * H * sizeof(float));
    
    // 3. Setup Manifest (The Thunk Context)
    TsfiZmmManifest m;
    memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_raymarch_svdag;
    // Only active registers: 0 (X), 1 (Y), 4 (Energy)
    m.active_mask = (1 << 0) | (1 << 1) | (1 << 4);
    
    // 4. Render Loop (Ray Bundles)
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x += 16) {
            float ray_x[16] __attribute__((aligned(512)));
            float ray_y[16] __attribute__((aligned(512)));
            float ray_e[16] __attribute__((aligned(512))) = {0}; // Energy
            
            for (int k = 0; k < 16; k++) {
                if (x + k < W) {
                    ray_x[k] = ((float)(x + k) / W) * 2.0f - 1.0f; // -1..1
                    ray_y[k] = ((float)y / H) * 2.0f - 1.0f;       // -1..1
                } else {
                    ray_x[k] = 10.0f; // Out of bounds
                    ray_y[k] = 10.0f;
                }
            }
            
            m.contiguous_rf = NULL; // Disable fast-path to use slots
            m.slots[0].data_ptr = ray_x;
            m.slots[1].data_ptr = ray_y;
            m.slots[4].data_ptr = ray_e; // Accumulator
            
            tsfi_dispatch_zmm_dynamic(&m);
            
            // B. Write Back
            for (int k = 0; k < 16; k++) {
                if (x + k < W) {
                    pixels[y * W + (x + k)] = ray_e[k];
                }
            }
        }
    }
    
    // 5. Output
    write_ppm("vision_test.ppm", W, H, pixels);
    
    lau_free(pixels);
    tsfi_svdag_destroy(dag);

    test_symmetric_vrs_verification();

    extern void tsfi_svdag_teardown(void);
    tsfi_svdag_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}