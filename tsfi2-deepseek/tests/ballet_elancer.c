#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

// Native reference for the JIT thunk
void __attribute__((noinline)) elancer_native(__m512 *out, const __m512 *a, const __m512 *b) {
    __m512 v_a = _mm512_load_ps(a);
    __m512 v_b = _mm512_load_ps(b);
    __m512 v_c = _mm512_set1_ps(0.5002f);
    
    // Increased Difficulty: Bounded mix simulating stable geometric synthesis
    __m512 res = _mm512_fmadd_ps(v_a, v_b, v_c);
    
    // Quick fractional bounding via reciprocal (1 / (1 + |res|))
    // We approximate absolute value with a max against -res, but since AVX512 doesn't have a direct abs,
    // we just square it and add 1.0f, then use rcp14
    __m512 sq = _mm512_fmadd_ps(res, res, _mm512_set1_ps(1.0f));
    res = _mm512_mul_ps(res, _mm512_rcp14_ps(sq)); // res / (res^2 + 1) -> bound between -0.5 and 0.5
    
    // Scale back up slightly and shift
    res = _mm512_fmadd_ps(res, _mm512_set1_ps(1.5f), _mm512_set1_ps(0.1f));
    
    _mm512_store_ps(out, res);
}

void elancer_benchmark(void *thunk) {
    const int iterations = 1000000;

    __m512 a __attribute__((aligned(64))) = _mm512_set1_ps(1.0f);
    __m512 b __attribute__((aligned(64))) = _mm512_set1_ps(2.0f);
    __m512 c __attribute__((aligned(64))) = _mm512_setzero_ps();

    void (*elancer_call)(__m512*, const __m512*, const __m512*) = (void (*)(__m512*, const __m512*, const __m512*))thunk;

    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Darting Vector Pipeline Active");
    }

    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        // ROUTINE: The Strike
        elancer_call(&c, &a, &b);
        elancer_call(&a, &c, &b);
        elancer_call(&b, &a, &c);
        
        // High-frequency live telemetry updates every 10,000 steps
        if (g_sys_telem && (i % 10000 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
            
            float tele_buf[16];
            _mm512_store_ps(tele_buf, c);
            uint64_t val;
            memcpy(&val, &tele_buf[0], sizeof(float)); // Copy float representation safely
            g_sys_telem->zmm_val = val; // Mirror the geometric bounds
        }
    }
    unsigned long long end = get_time_ns();

    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }

    double total_ops = (double)iterations * 3.0 * 5.0; // 3 calls per iter, 5 ops per call
    double total_floats = total_ops * 16.0;
    double duration_s = (double)(end - start) / 1e9;
    double gflops = (total_floats / duration_s) / 1e9;

    printf("[BENCH] Élancer Throughput: %.2f GFLOPS (Complex AVX-512 Pipeline).\n", gflops);
    
    float result_buffer[16];
    _mm512_store_ps(result_buffer, c);
    printf("  [ROUTINE] Verification: C[0] morphed to %.4f\n", result_buffer[0]);
    // It should be bound and finite
    assert(result_buffer[0] >= -2.0f && result_buffer[0] <= 2.0f);
}

void test_elancer_logic() {
    printf("[SUB-TEST] Élancer (Darting) - Thunk Vector Routine\n");
    size_t cp = BALLET_CHECKPOINT();

    printf("  [ROUTINE] 1. Creating Thunk Proxy for Vector Math...\n");
    ThunkProxy *proxy = ThunkProxy_create();

    printf("  [ROUTINE] 2. Emitting AVX-512 JIT Logic (Add -> Mul -> Store)...\n");
    // Emit a straight forwarding thunk without baking arguments
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)elancer_native, 0);

    printf("  [ROUTINE] 3. Sealing Proxy for High-Throughput Execution...\n");
    ThunkProxy_seal(proxy);

    printf("[INFO] Executing AVX-512 vector darts via Thunk Manifold...\n");
    elancer_benchmark(thunk);

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Élancer failed: System integrity fractured.\n");
        exit(1);
    }

    printf("[INFO] Élancer Task is Finished. ALU lanes strike. 16 floats finish.\n");

    printf("  [ROUTINE] 4. Reclaiming Vector Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);

    BALLET_AUDIT(cp, "Elancer Vector Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 6: Élancer (Darting) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step6_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_elancer_logic();

    printf("=== Élancer Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step6_telem");
    return 0;
}
