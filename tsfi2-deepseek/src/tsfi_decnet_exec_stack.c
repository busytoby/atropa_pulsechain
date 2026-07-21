#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_decnet_exec_stack.h"
#include "tsfi_lowpower_fet.h"

int tsfi_decnet_exec_stack_init(
    uint32_t decnet_proc_id,
    tsfi_decnet_exec_stack_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_decnet_exec_stack_t));

    proc->decnet_proc_id = decnet_proc_id;
    proc->evm_gas_units = 200; // 200 Gas / Auncient Ether Units per DECnet packet frame slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "DECNET_EXEC_STACK_%08X.DAT.BIN", decnet_proc_id);
    return 0;
}

int tsfi_decnet_exec_stack_push_frame(
    tsfi_decnet_exec_stack_t *proc,
    uint8_t src_area, uint16_t src_node,
    uint8_t dst_area, uint16_t dst_node,
    const uint8_t *payload, size_t payload_len
) {
    if (!proc || !payload || payload_len > sizeof(proc->payload)) return -1;

    proc->last_hdr.src_area = src_area;
    proc->last_hdr.src_node = src_node;
    proc->last_hdr.dst_area = dst_area;
    proc->last_hdr.dst_node = dst_node;
    proc->last_hdr.nsp_flags = 0x01; // DATA_CTL
    proc->last_hdr.seq_num++;

    memcpy(proc->payload, payload, payload_len);
    proc->payload_len = payload_len;

    printf("[DECNET EXEC STACK] Proc: %u | Route: %u.%u -> %u.%u | Seq: %u | Payload: %zu Bytes | Gas: %u | File: %s\n",
           proc->decnet_proc_id, src_area, src_node, dst_area, dst_node,
           proc->last_hdr.seq_num, payload_len, proc->evm_gas_units, proc->tape_dat_bin);

    return 0; // DECnet stack push successful
}
