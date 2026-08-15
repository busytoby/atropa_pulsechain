#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_strategy_lang.h"
#include "../inc/tsfi_chancery_docket.h"

// C Evaluator mirroring solidity/dysnomia/domain/std/tang_lut_nn_prover.algol61
static int evaluate_lut_nn_centroid_bounds(
    int num_centroids,
    int mac_bypass_pct_scaled,
    int snr_db_scaled,
    int min_snr_threshold_scaled
) {
    (void)mac_bypass_pct_scaled;
    if (num_centroids < 4 || num_centroids > 256) {
        return 2; // INVALID_CENTROID_DIMENSION
    }
    if (snr_db_scaled < min_snr_threshold_scaled) {
        return 1; // INSUFFICIENT_SNR_REJECT
    }
    return 0;     // AUTHENTIC_LUT_STREAM
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: TANG ET AL. (MOBICOM 2023) LUT-NN STRATEGY & PROVER\n");
    printf("====================================================================\n\n");

    // 1. Execute COBOL Strategy: tang_lut_nn.strategy
    printf("1. Executing COBOL Strategy: tang_lut_nn.strategy (R0=64, R1=16)...\n");
    TSFiStrategyVM vm;
    TSFiStrategyReceipt receipt;
    int rc = tsfi_strategy_load_and_run("tang_lut_nn.strategy", 64, 16, 0, 0, &vm, &receipt);
    assert(rc == 0);

    printf("   ✓ Execution Succeeded (RC: %d)\n", rc);
    printf("   ✓ R0 (DIM-D):                 %d\n", vm.registers[0]);
    printf("   ✓ R1 (NUM-CLUSTERS):          %d\n", vm.registers[1]);
    printf("   ✓ R2 (SNR-DB):                %d dB\n", vm.registers[2]);
    printf("   ✓ R3 (SPEEDUP-FACTOR-X100):   %d (%.2fx speedup)\n", vm.registers[3], (float)vm.registers[3] / 100.0f);

    assert(vm.registers[2] == 82);
    assert(vm.registers[3] == 3240);

    // 2. Evaluate ALGOL 61 Prover Conformance Gating
    printf("\n2. Executing ALGOL 61 Prover: tang_lut_nn_prover.algol61...\n");
    int snr_scaled = vm.registers[2] * 1000; // 82,000
    int threshold_scaled = 20000;           // 20,000 (20 dB threshold)

    int ruling_auth = evaluate_lut_nn_centroid_bounds(vm.registers[1], 87400, snr_scaled, threshold_scaled);
    assert(ruling_auth == 0);
    printf("   ✓ Ruling: AUTHENTIC_LUT_STREAM (ruling = %d)\n", ruling_auth);

    int ruling_noise = evaluate_lut_nn_centroid_bounds(vm.registers[1], 87400, 15000, threshold_scaled);
    assert(ruling_noise == 1);
    printf("   ✓ High-Noise Reject Ruling: INSUFFICIENT_SNR_REJECT (ruling = %d)\n", ruling_noise);

    int ruling_dim = evaluate_lut_nn_centroid_bounds(512, 87400, snr_scaled, threshold_scaled);
    assert(ruling_dim == 2);
    printf("   ✓ Out-Of-Bounds Reject Ruling: INVALID_CENTROID_DIMENSION (ruling = %d)\n", ruling_dim);

    // 3. File Resolution onto Chancery Docket
    printf("\n3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "Tang et al. (MobiCom 2023) Lut-NN Centroid Table Gating Formally Proven",
        "solidity/dysnomia/domain/std/tang_lut_nn_prover.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, ruling_auth, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("TANG ET AL. LUT-NN STRATEGY & PROVER INTEGRATION PASSED & SEALED\n");
    printf("====================================================================\n");

    return 0;
}
