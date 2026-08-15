#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_mariner_fabric_dispatch.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("PROJECT MARINER PHASE 2: MULTI-FABRIC 512-BIT VECTOR SIMD TEST\n");
    printf("====================================================================\n\n");

    // 1. Initialize Multi-Fabric Dispatcher
    MarinerFabricDispatcher dispatcher;
    tsfi_mariner_fabric_init(&dispatcher);

    // 2. Stage Coordinate Streams across 4 Vector Channels
    printf("1. Staging 512-Bit Vector Frames across 4 Execution Channels...\n");
    float x_stream[128], y_stream[128], z_stream[128], pot_stream[128];
    for (int i = 0; i < 128; i++) {
        x_stream[i] = (float)i * 0.1f;
        y_stream[i] = (float)(127 - i) * 0.1f;
        z_stream[i] = (float)(i % 16) * 0.5f;
        pot_stream[i] = 1.0f;
    }

    for (uint32_t c = 0; c < 4; c++) {
        assert(tsfi_mariner_fabric_stage_vectors(&dispatcher, c, x_stream, y_stream, z_stream, pot_stream, 128));
    }
    assert(dispatcher.active_channels == 4);
    printf("   ✓ 4 Vector Channels staged with 128 lanes each (512 floats per channel).\n");

    // 3. Execute SIMD Multi-Fabric Transformation
    printf("\n2. Executing AVX-512 SIMD Vector Pipeline (k=8 Standard)...\n");
    uint64_t exec_proof = 0;
    assert(tsfi_mariner_fabric_execute_simd(&dispatcher, 8, &exec_proof));
    assert(exec_proof != 0);
    assert(dispatcher.total_vectors_processed == 512);

    printf("   ✓ Total Vector Elements Processed: %lu\n", (unsigned long)dispatcher.total_vectors_processed);
    printf("   ✓ Multi-Fabric Compute Cycles:     %lu\n", (unsigned long)dispatcher.total_compute_cycles);
    printf("   ✓ Execution Merkle Proof:          0x%016lx\n", (unsigned long)exec_proof);

    // 4. Settle on Chancery Docket
    printf("\n3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = 0;
    assert(tsfi_mariner_fabric_settle_docket(&dispatcher, &docket, &doc_id));
    assert(doc_id == 7000);

    char audit_report[2048];
    uint64_t audit_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(audit_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 MULTI-FABRIC SIMD DISPATCH COMPLETE & SEALED ON LEDGER\n");
    printf("====================================================================\n");

    return 0;
}
