#ifndef TSFI_HARDWARE_H
#define TSFI_HARDWARE_H

#include <stdint.h>

// TSFi2 Hardware Definitions (Navi 48)
// Protocol: TSFI_DOORBELL_PROTOCOL.md

// -----------------------------------------------------------------------------
// BAR 0: VRAM (High Bandwidth Memory)
// -----------------------------------------------------------------------------
// Usage: Packet Storage, Texture Data, Framebuffer, Resident Kernels
// Access: Direct Physical Mapping (via lau_memory thunks)
#define TSFI_GPU_BAR0_BASE      0xf800000000ULL
#define TSFI_GPU_BAR0_SIZE      (16ULL * 1024 * 1024 * 1024) // 16GB
#define TSFI_GPU_BAR0_ALIGN     4096 // Page Alignment

// -----------------------------------------------------------------------------
// BAR 2: Doorbell Aperture
// -----------------------------------------------------------------------------
// Usage: The ONLY hardware trigger. "One Ring" bootstrap mechanism.
// Access: Write-Only (typically)
#define TSFI_GPU_DOORBELL_BASE  0xfc00000000ULL
#define TSFI_GPU_DOORBELL_SIZE  (256 * 1024 * 1024) // 256MB
#define TSFI_DOORBELL_STRIDE    4096 // 4KB per Queue

// -----------------------------------------------------------------------------
// BAR 5: MMIO Registers (Legacy)
// -----------------------------------------------------------------------------
// STATUS: REJECTED / FORBIDDEN
// Access to this region is strictly prohibited by TSFi Zhong architecture.
// All control must occur via PM4 packets in BAR 0.
// #define TSFI_GPU_MMIO_BASE  0xdd800000 // DO NOT USE

#endif // TSFI_HARDWARE_H
