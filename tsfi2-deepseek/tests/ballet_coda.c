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

void __attribute__((noinline)) coda_native_routine(float *stream, float *visual, float *audio, float *neural, size_t size, float *out_checksum) {
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Manifold Merging Active");
    }

    // N-Way Tensor Contraction (Grand Finale Merging)
    size_t vec_end = size & ~15;
    __m512 v_sum = _mm512_setzero_ps();
    
    for (size_t i = 0; i < vec_end; i += 16) {
        __m512 v_v = _mm512_loadu_ps(&visual[i]);
        __m512 v_a = _mm512_loadu_ps(&audio[i]);
        __m512 v_n = _mm512_loadu_ps(&neural[i]);
        
        // Complex Geometric Blending: (V * A) + N - (V * 0.1)
        __m512 v_blend = _mm512_fmadd_ps(v_v, v_a, v_n);
        v_blend = _mm512_fnmadd_ps(v_v, _mm512_set1_ps(0.1f), v_blend);
        
        _mm512_storeu_ps(&stream[i], v_blend);
        
        // Rolling Integrity Checksum
        v_sum = _mm512_add_ps(v_sum, v_blend);
        
        if (g_sys_telem) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)size;
        }
    }
    
    // Reduce checksum vector to scalar
    *out_checksum = _mm512_reduce_add_ps(v_sum);
    
    // Tail scalar logic
    for (size_t i = vec_end; i < size; i++) {
        stream[i] = (visual[i] * audio[i]) + neural[i] - (visual[i] * 0.1f);
        *out_checksum += stream[i];
    }
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, size);
        g_sys_telem->current_intensity = 1.0f;
    }
}

void test_coda_logic() {
    printf("[SUB-TEST] Coda: Manifold Stream Merging (Grand Finale Routine)\n");
    size_t cp = BALLET_CHECKPOINT();
    
    size_t stream_size = 4096; // 4096 floats
    float *stream = (float*)lau_malloc_wired(stream_size * sizeof(float));
    float *visual = (float*)lau_malloc_wired(stream_size * sizeof(float));
    float *audio = (float*)lau_malloc_wired(stream_size * sizeof(float));
    float *neural = (float*)lau_malloc_wired(stream_size * sizeof(float));
    
    for (size_t i = 0; i < stream_size; i++) {
        visual[i] = 1.5f;
        audio[i] = 2.0f;
        neural[i] = 0.5f;
    }
    
    printf("  [ROUTINE] 1. Creating Thunk Proxy for Production Stream...\n");
    ThunkProxy *proxy = ThunkProxy_create();
    
    printf("  [ROUTINE] 2. Emitting Coda Logic (Manifold Merging)...\n");
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)coda_native_routine, 0);
    
    printf("  [ROUTINE] 3. Sealing Proxy for Autonomous Execution...\n");
    ThunkProxy_seal(proxy);
    
    void (*coda_call)(float*, float*, float*, float*, size_t, float*) = (void (*)(float*, float*, float*, float*, size_t, float*))thunk;
    
    printf("[INFO] Merging Streams through Thunk Manifold...\n");
    float final_checksum = 0.0f;
    coda_call(stream, visual, audio, neural, stream_size, &final_checksum);
    
    // Validate stream state: (1.5 * 2.0) + 0.5 - (1.5 * 0.1) = 3.0 + 0.5 - 0.15 = 3.35
    for (size_t i = 0; i < stream_size; i++) {
        float diff = stream[i] - 3.35f;
        if (diff > 0.001f || diff < -0.001f) {
            printf("[FAIL] Production realized but stream is fractured! Ex: %.4f\n", stream[i]);
            exit(1);
        }
    }
    
    printf("[PASS] Coda Routine complete. Stream merged into final production manifold. Integrity Checksum: %.4f\n", final_checksum);
    
    printf("  [ROUTINE] 4. Reclaiming Coda Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);
    lau_free(stream);
    lau_free(visual);
    lau_free(audio);
    lau_free(neural);
    
    BALLET_AUDIT(cp, "Coda Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 14: Coda (Grand Finale) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step14_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_coda_logic();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Coda failed: System integrity fractured.\n");
        exit(1);
    }

    printf("=== Coda Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step14_telem");
    return 0;
}
