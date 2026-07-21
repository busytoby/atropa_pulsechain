#include "tsfi_parc_ingerman_thunk.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static uint64_t dynamic_scsi_register_evaluator(void *ctx) {
    uint64_t *base_val = (uint64_t *)ctx;
    return (*base_val) * 2 + 30; // Keycode 30 (a/A) shift
}

static uint64_t lissajous_phase_evaluator(void *ctx) {
    uint64_t *step = (uint64_t *)ctx;
    return (*step) * 953467954114363ULL; // MotzkinPrime lookup
}

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER Z INGERMAN CALL-BY-NAME THUNK AUDIT     \n");
    printf("=======================================================\n");

    tsfi_ingerman_thunk_manager_t mgr;
    assert(tsfi_ingerman_thunk_manager_initialize(&mgr) == 0);
    printf("[PASS] Ingerman Thunk Manager Initialized\n");

    uint64_t scsi_state = 100;
    uint64_t lissajous_step = 5;

    uint32_t thunk1, thunk2;
    assert(tsfi_ingerman_thunk_register(&mgr, "thunk_scsi_reg", dynamic_scsi_register_evaluator, &scsi_state,
                                         "dynamic_0x323053435349526567", &thunk1) == 0);
    assert(tsfi_ingerman_thunk_register(&mgr, "thunk_lissajous_phase", lissajous_phase_evaluator, &lissajous_step,
                                         "dynamic_0x4c697373616a6f7573", &thunk2) == 0);
    printf("[PASS] Registered 2 Call-by-Name Thunks (SCSI Register & Lissajous Phase)\n");

    // Evaluate Thunk 1 dynamically
    uint64_t val1 = tsfi_ingerman_thunk_evaluate(&mgr, thunk1);
    assert(val1 == 230); // 100 * 2 + 30
    printf("[PASS] Thunk 1 Call-by-Name Evaluation: %lu (Expected: 230)\n", val1);

    // Mutate environment variable context and re-evaluate dynamically by name
    scsi_state = 200;
    uint64_t val1_new = tsfi_ingerman_thunk_evaluate_by_name(&mgr, "thunk_scsi_reg");
    assert(val1_new == 430); // 200 * 2 + 30
    printf("[PASS] Thunk 1 Dynamic Context Re-evaluation: %lu (Expected: 430)\n", val1_new);

    // Evaluate Thunk 2
    uint64_t val2 = tsfi_ingerman_thunk_evaluate_by_name(&mgr, "thunk_lissajous_phase");
    assert(val2 == 5 * 953467954114363ULL);
    printf("[PASS] Thunk 2 Evaluation: %lu\n", val2);

    printf("=======================================================\n");
    printf(" ALL PETER Z INGERMAN THUNK TESTS PASSED (100%%)       \n");
    printf("=======================================================\n");

    return 0;
}
