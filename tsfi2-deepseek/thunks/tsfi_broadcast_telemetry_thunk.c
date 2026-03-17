#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include <immintrin.h>

/**
 * TSFi Firmware Standard Cell: Broadcast Telemetry Thunk
 * 
 * Physically replaces abstract POSIX software clock load guessing.
 * This standard cell intercepts the active `LauOccupancyManifest` mapped 
 * in the ReBAR pool and physically aggregates the explicit hardware 
 * thread pressure across the 64 Workgroup Processors (WGP) utilizing 
 * strictly deterministic AVX-512 vector sweeps.
 */
void tsfi_broadcast_telemetry_thunk(LauRegisterBank* regs) {
    if (!regs) return;

    // SGPR Boundary Mapping
    // sgpr[10]/[11]: Pointer to LauOccupancyManifest
    uintptr_t manifest_ptr = ((uintptr_t)regs->sgpr[11] << 32) | (uintptr_t)regs->sgpr[10];
    LauOccupancyManifest* manifest = (LauOccupancyManifest*)manifest_ptr;

    if (!manifest) {
        regs->sgpr[0] = 0; // Hardware Fault: Null Manifest
        return;
    }

    // Zero-Copy Hardware Metric Aggregation
    // The manifest tracks 64 WGP units as a continuous 256-byte array of 32-bit integers.
    // We ingest the pressure via four rapid AVX-512 geometric sweeps.
    __m512i sum_vec = _mm512_setzero_si512();

    sum_vec = _mm512_add_epi32(sum_vec, _mm512_loadu_si512((__m512i*)&manifest->wgp_active_waves[0]));
    sum_vec = _mm512_add_epi32(sum_vec, _mm512_loadu_si512((__m512i*)&manifest->wgp_active_waves[16]));
    sum_vec = _mm512_add_epi32(sum_vec, _mm512_loadu_si512((__m512i*)&manifest->wgp_active_waves[32]));
    sum_vec = _mm512_add_epi32(sum_vec, _mm512_loadu_si512((__m512i*)&manifest->wgp_active_waves[48]));

    // Reduce the accumulated vector into a singular scalar load metric
    uint32_t total_wgp_pressure = _mm512_reduce_add_epi32(sum_vec);

    // Compute active load percentage (Assuming max physical pressure of 1024 active waves)
    float physical_load_ratio = (float)total_wgp_pressure / 1024.0f;
    if (physical_load_ratio > 1.0f) physical_load_ratio = 1.0f;
    
    // Convert to a scaled integer representation (0 - 10000 -> 0.00% - 100.00%) for transport
    uint32_t cpu_load_scaled = (uint32_t)(physical_load_ratio * 10000.0f);

    // Aggregate physical ACE (Asynchronous Compute Engine) pressure
    uint32_t total_ace_pressure = 0;
    for (int i = 0; i < 8; i++) {
        total_ace_pressure += manifest->ace_pressure[i];
    }
    
    // Map raw GPU metric (Assuming max 128 queue slots)
    float ace_load_ratio = (float)total_ace_pressure / 128.0f;
    if (ace_load_ratio > 1.0f) ace_load_ratio = 1.0f;
    uint32_t gpu_load_scaled = (uint32_t)(ace_load_ratio * 10000.0f);

    // Export natively calculated topological load metrics
    regs->sgpr[0] = 1; // Success
    regs->sgpr[1] = cpu_load_scaled;
    regs->sgpr[2] = gpu_load_scaled;
}