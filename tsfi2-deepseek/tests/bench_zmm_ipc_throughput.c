#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <immintrin.h>

#define TARGET_MB 130
#define CHUNK_SIZE 128 // 128 bytes per batch
#define ITERATIONS ((TARGET_MB * 1024 * 1024) / CHUNK_SIZE)

typedef struct {
    LauWireFirmware_State rtl;
    _Atomic uint32_t write_count;
    _Atomic uint32_t read_count;
} SharedZMM;

int main() {
    tsfi_io_printf(stdout, "=== TSFi ZMM IPC Throughput Benchmark ===\n");
    tsfi_io_printf(stdout, "Targeting %d MB across isolated process boundary...\n", TARGET_MB);

    SharedZMM* shared = (SharedZMM*)mmap(NULL, sizeof(SharedZMM), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) return 1;
    memset(shared, 0, sizeof(SharedZMM));

    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    pid_t pid = fork();

    if (pid == 0) {
        // --- PRODUCER (Firmware) ---
        struct YI* root_yi = tsfi_reaction_shoot(Prime);
        TSFiBigInt* Pi = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
        struct Dai* dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi, NULL);

        for (int i = 0; i < ITERATIONS; i++) {
            // Wait for reader to catch up (simple spinlock to prevent overrun)
            while (atomic_load_explicit(&shared->write_count, memory_order_acquire) > 
                   atomic_load_explicit(&shared->read_count, memory_order_acquire)) {
                // spin
            }

            // Write 128 bytes (2x 512-bit vectors) directly into ZMM mirror
            _mm512_storeu_si512(&shared->rtl.wrf[120], _mm512_loadu_si512((__m512i*)dai->Ichidai->limbs));
            _mm512_storeu_si512(&shared->rtl.wrf[121], _mm512_loadu_si512((__m512i*)dai->Daiichi->limbs));
            
            atomic_fetch_add_explicit(&shared->write_count, 1, memory_order_release);
        }

        freeDAI(dai); freeYI(root_yi);
        exit(0);

    } else {
        // --- CONSUMER (Cockpit) ---
        uint64_t start_time = get_time_ns() / 1000000;
        
        for (int i = 0; i < ITERATIONS; i++) {
            // Wait for writer
            while (atomic_load_explicit(&shared->read_count, memory_order_acquire) >= 
                   atomic_load_explicit(&shared->write_count, memory_order_acquire)) {
                // spin
            }

            // Read 128 bytes
            __m512i read_ichi = _mm512_loadu_si512(&shared->rtl.wrf[120]);
            __m512i read_dai = _mm512_loadu_si512(&shared->rtl.wrf[121]);
            
            uint64_t temp_ichi[8];
            uint64_t temp_dai[8];
            _mm512_storeu_si512((__m512i*)temp_ichi, read_ichi);
            _mm512_storeu_si512((__m512i*)temp_dai, read_dai);
            
            // Prevent optimization out
            if (temp_ichi[0] == 0 && temp_dai[0] == 0) {
                tsfi_io_printf(stderr, "Null read detected.\n");
            }

            atomic_fetch_add_explicit(&shared->read_count, 1, memory_order_release);
        }

        uint64_t end_time = get_time_ns() / 1000000;
        double duration_ms = (double)(end_time - start_time);
        if (duration_ms == 0) duration_ms = 1;
        
        double mb_per_sec = (TARGET_MB / (duration_ms / 1000.0));
        
        tsfi_io_printf(stdout, "\n[RESULTS]\n");
        tsfi_io_printf(stdout, "Transferred    : %d MB\n", TARGET_MB);
        tsfi_io_printf(stdout, "Duration       : %.2f ms\n", duration_ms);
        tsfi_io_printf(stdout, "IPC Throughput : %.2f MB/sec\n", mb_per_sec);

        waitpid(pid, NULL, 0);
    }

    tsfi_bn_free(Prime);
    munmap(shared, sizeof(SharedZMM));

    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

        
    return 0;
}