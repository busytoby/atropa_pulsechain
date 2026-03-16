#ifndef LAU_WIRE_MCP_H
#define LAU_WIRE_MCP_H

#include "tsfi_types.h"
#include "lau_wire_log.h"
#include <stdatomic.h>

/**
 * @brief LauMasterWavefront: The Unified Dielectric Manifold
 * 
 * Supersedes dedicated log and command wavefronts.
 * Established as the singular Host-to-Plugin command and data channel.
 */
typedef struct {
    _Atomic uint32_t lock;      // Singular lock for the entire manifold

    // --- Control Plane (Transactional) ---
    uint32_t command_id;        // 0: Idle, 1: Read, 2: Write, 3: Thunk, 4: Flush
    uint64_t arg0;
    uint64_t arg1;
    uint64_t result;
    uint32_t status;            // 0: Ready, 1: Busy, 2: Done, 3: Error

    // --- Data Plane (Streaming) ---
    LauWireLog stdin_log;
    LauWireLog stdout_log;

    // --- Neural Metadata ---
    uint64_t pulse_count;
    float resonance_intensity;
} LauMasterWavefront;

#endif // LAU_WIRE_MCP_H
