#ifndef TSFI_HELMHOLTZ_DISPATCHER_H
#define TSFI_HELMHOLTZ_DISPATCHER_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_autodin_zmm_tx.h"

typedef struct {
    int yul_executed;
    int autodin_mined;
    int scsi_handshaked;
    int tape_inscribed;
    int vulkan_projected;
    char global_tx_hash[65];
} tsfi_helmholtz_dispatch_result_t;

/* Dispatch a unified Helmholtz wave state cycle across all 5 isomorphic domains */
int tsfi_helmholtz_dispatch_wave_cycle(
    uint32_t selector,
    const char *target_contract,
    uint8_t scsi_keycode,
    const char *tape_path,
    uint8_t clearance_level,
    tsfi_helmholtz_dispatch_result_t *out_result
);

#endif // TSFI_HELMHOLTZ_DISPATCHER_H
