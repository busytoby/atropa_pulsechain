#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_ar.h"

// Define matching local structure of xcom_receiver_t to verify memory alignment
typedef enum {
    LOCAL_CUTOFF_STATE,
    LOCAL_CONDUC_STATE
} local_gate_state_t;

typedef struct {
    uint32_t expected_seq;
    uint32_t last_nonce;
    local_gate_state_t write_gate;
    char rx_buffer[64];
} local_xcom_receiver_t;

int main(void) {
    printf("=== RUNNING AUNCIENT USD ASSET RESOLVER (AR) TESTS ===\n");

    TSFiAssetResolver ar;
    tsfi_ar_init(&ar);

    char resolved[128];

    // 1. Resolve custom asset URI scheme
    bool ok = tsfi_ar_resolve(&ar, "auncient://terrain_map", resolved);
    assert(ok == true);
    printf("   Resolved URI: %s\n", resolved);
    assert(strcmp(resolved, "/root/assets/terrain_map.dat.bin") == 0);

    // 2. Resolve invalid scheme
    ok = tsfi_ar_resolve(&ar, "http://invalid_path", resolved);
    assert(ok == false);

    // 3. Integrate with XCOM receiver gate
    local_xcom_receiver_t rx = {0};
    rx.write_gate = LOCAL_CUTOFF_STATE;

    tsfi_ar_integrate_xcom(true, &rx);
    assert(rx.write_gate == LOCAL_CONDUC_STATE);

    tsfi_ar_integrate_xcom(false, &rx);
    assert(rx.write_gate == LOCAL_CUTOFF_STATE);

    printf("   ✓ Custom scheme resolution to .dat.bin verified successfully.\n");
    printf("   ✓ XCOM receiver gate synchronization verified successfully.\n");
    printf("=== AUNCIENT USD ASSET RESOLVER (AR) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
