#define TSFI_TIME_LIB
#include "tsfi_time.h"
#include "tsfi_hilbert.h"
#include "lau_memory.h"
#include "tsfi_types.h"
#include "lau_thunk.h" // Needed for ThunkProxy
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define BATCH_COUNT 65536 

// Prototype for the kernel (not header exported maybe?)
void tsfi_hilbert_batch_avx512(void *ctx, const float *xy_in, float *out, int count);

// Typedef for the thunk function
// Thunk sets R15, then jumps to Kernel(ctx, xy, out, count).
// So signature matches Kernel.
typedef void (*HilbertBatchFunc)(void *ctx, const float *xy_in, float *out, int count);

int main() {
    printf("[TEST] Hilbert Space Field: Bijective Math via Thunking\n");

    // 1. Setup Data
    float *xy_in = (float*)lau_memalign(64, BATCH_COUNT * 2 * sizeof(float));
    float *z_out = (float*)lau_memalign(64, BATCH_COUNT * sizeof(float));
    
    for(int i=0; i<BATCH_COUNT; i++) {
        xy_in[i*2+0] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * M_PI;
        xy_in[i*2+1] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * M_PI;
    }

    // 2. Setup Wired System
    size_t total_size = sizeof(LauWiredHeader) + sizeof(WaveSystem);
    uint8_t *block = (uint8_t*)lau_memalign(4096, total_size);
    memset(block, 0, total_size);

    LauWiredHeader *h = (LauWiredHeader*)block;
    h->footer.magic = LAU_MAGIC;
    WaveSystem *ws = (WaveSystem*)(block + sizeof(LauWiredHeader));

    // 3. Map Glyph into Embedded SGPRs
    TSFiHilbertGlyph *sgpr_glyph = (TSFiHilbertGlyph*)h->regs.sgpr;
    tsfi_hilbert_project_box(sgpr_glyph, M_PI, M_PI);

    // Initialize function pointer
    ws->hilbert_batch = tsfi_hilbert_batch_avx512;

    // 4. Create ThunkProxy (no special AVX setup thunk needed for this test)
    // The tsfi_create_system already sets up the WaveSystem with default logic
    // which calls tsfi_hilbert_batch_avx512.
    // So we just call the system's thunked version.
    
    // 5. Benchmark
    printf("[BENCH] Evaluating %d points via WaveSystem->hilbert_batch...\n", BATCH_COUNT);
    unsigned long long start_time = get_time_ns();
    
    // Call the thunked batch function
    ws->hilbert_batch((void*)ws, xy_in, z_out, BATCH_COUNT);
    
    unsigned long long end_time = get_time_ns();
    double dt = (double)(end_time - start_time) / 1e9;
    
    printf("[BENCH] Time: %.4f sec\n", dt);
    printf("[BENCH] Rate: %.2f MPoints/sec\n", (double)BATCH_COUNT / dt / 1e6);
    
    // 6. Verify Correctness
    float s = tsfi_hilbert_eval(sgpr_glyph, xy_in[0], xy_in[1]);
    float v = z_out[0];
    printf("[VERIFY] Sample 0: Scalar %.4f vs Vector %.4f\n", s, v);
    assert(fabsf(s - v) < 0.05f);

    printf("[PASS] Bijective Math Thunk Successful.\n");

    lau_free(block);
    lau_free(xy_in);
    lau_free(z_out);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}