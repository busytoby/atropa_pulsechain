#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "tsfi_yang_registry.h"
#include "tsfi_wavelet_arena.h"

// Simulate the XOR-fold mass measurement
static uint64_t measure_acoustic_mass(const uint8_t *buffer, size_t len) {
    uint64_t mass = 0x123456789ABCDEF0;
    for (size_t i = 0; i < len; i++) {
        mass ^= ((uint64_t)buffer[i] << ((i % 8) * 8));
        mass *= 1099511628211ULL;
    }
    return mass;
}

int test_registry_mapping() {
    printf("\n[TEST 1] Shared Memory Acoustic Registry\n");
    int fd = shm_open("/tsfi_test_acoustic_space", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        printf("[FAIL] Failed to create shm\n");
        return 1;
    }
    ftruncate(fd, sizeof(TsfiAcousticRegistry));
    
    TsfiAcousticRegistry *reg = mmap(NULL, sizeof(TsfiAcousticRegistry), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (reg == MAP_FAILED) {
        printf("[FAIL] Failed to mmap shm\n");
        close(fd);
        return 1;
    }

    // Set telemetry states
    reg->pdai_manifold_pressure = 0.85f;
    reg->yang_heartbeat_hz = 72.5f;
    reg->yang_static_amplitude = 0.4f;
    reg->agent_participation_flag = 1;
    reg->target_genome_id = 3;
    strcpy(reg->agent_monologue, "The acoustic pressure is rising. I must shift to the fetal genome.");

    // Validate states
    printf("  -> Written pDAI Pressure: %.2f\n", reg->pdai_manifold_pressure);
    printf("  -> Written Heartbeat: %.2f Hz\n", reg->yang_heartbeat_hz);
    printf("  -> Agent Target Genome: %d\n", reg->target_genome_id);
    printf("  -> Agent Monologue: %s\n", reg->agent_monologue);

    if (reg->target_genome_id != 3) {
        printf("[FAIL] Invalid genome ID readback.\n");
        return 1;
    }

    munmap(reg, sizeof(TsfiAcousticRegistry));
    close(fd);
    shm_unlink("/tsfi_test_acoustic_space");
    printf("[PASS] Registry Mapping Verified.\n");
    return 0;
}

int test_acoustic_buffer() {
    printf("\n[TEST 2] Wavelet Acoustic Pulse Buffer\n");
    TsfiAcousticBuffer buf = {0};
    
    TsfiWavelet mock_wavelets[10];
    memset(mock_wavelets, 0, sizeof(mock_wavelets));
    for(int i=0; i<10; i++) {
        tsfi_BufferPulse(&buf, &mock_wavelets[i], i % 2, i * 1000, i * 2000);
    }
    
    printf("  -> Buffered Pulses: %d/256\n", buf.count);
    if (buf.count != 10) {
        printf("[FAIL] Buffer count mismatch. Expected 10, got %d\n", buf.count);
        return 1;
    }
    
    for(int i=0; i<buf.count; i++) {
        if (buf.queue[i].alpha_proof != (uint64_t)(i * 1000) || buf.queue[i].beta_payment != (uint64_t)(i * 2000)) {
            printf("[FAIL] Pulse data corruption at index %d\n", i);
            return 1;
        }
    }
    
    tsfi_CommitBuffer(&buf, 999);
    printf("  -> Buffer committed. Count reset to: %d\n", buf.count);
    if (buf.count != 0) {
        printf("[FAIL] Buffer failed to clear on commit.\n");
        return 1;
    }

    printf("[PASS] Acoustic Buffer verified.\n");
    return 0;
}

int benchmark_topological_resonance() {
    printf("\n[TEST 3] Lockless Topological Resonance Benchmark\n");
    
    uint8_t dynamic_memory[1024];
    memset(dynamic_memory, 0, sizeof(dynamic_memory));
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    uint64_t base_mass = measure_acoustic_mass(dynamic_memory, sizeof(dynamic_memory));
    printf("  -> Base Topological Mass: 0x%016lx\n", base_mass);
    
    int detected_shifts = 0;
    for (int i = 0; i < 10000; i++) {
        // Mutate memory (guaranteed non-colliding increment)
        dynamic_memory[(i * 7) % 1024] += 1;
        
        // Measure mass locklessly
        uint64_t new_mass = measure_acoustic_mass(dynamic_memory, sizeof(dynamic_memory));
        if (new_mass != base_mass) {
            base_mass = new_mass;
            detected_shifts++;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    printf("  -> Detected %d resonance shifts.\n", detected_shifts);
    printf("  -> Benchmark completed in %.2f ms (%.2f ops/sec)\n", duration, 10000.0 / (duration / 1000.0));
    
    if (detected_shifts != 10000) {
        printf("[FAIL] Topological shift tracking lost mutations.\n");
        return 1;
    }

    printf("[PASS] Topological Resonance locked and measured.\n");
    return 0;
}

int main() {
    printf("=== TSFi Total Acoustic Telemetry Architecture ===\n");
    
    int errors = 0;
    errors += test_registry_mapping();
    errors += test_acoustic_buffer();
    errors += benchmark_topological_resonance();
    
    if (errors > 0) {
        printf("\n[CRITICAL] %d Acoustic Telemetry systems fractured.\n", errors);
        return 1;
    }
    
    printf("\n[SUCCESS] Total Acoustic Telemetry is RIGID.\n");
    return 0;
}
