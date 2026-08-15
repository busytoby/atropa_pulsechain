#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_mariner_biological_receptor.h"
#include "../inc/tsfi_zmm_vm.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("MARINER PHASE 2: BIOLOGICAL RECEPTOR TO ZMM HARDWARE REGISTER BINDING\n");
    printf("====================================================================\n\n");

    // 1. Ingest and Transduce Sequence State
    MarinerBiologicalState bio_state;
    tsfi_mariner_bio_init(&bio_state);

    const char *seq = "MALWMRLLPLLALLALWGPDPAAAFVNQHLCGSHLVEALYLVCGERGFFYTPKTRREAEDLQVGQVELGGG";
    assert(tsfi_mariner_bio_ingest_sequence(&bio_state, 8006, seq));
    assert(tsfi_mariner_bio_transduce_wave512_simd(&bio_state, 8));
    assert(bio_state.is_receptor_bound);

    printf("1. Biological Sequence Transduced:\n");
    printf("   Sequence ID:      %u\n", bio_state.sequence_id);
    printf("   Receptor Nodes:   %u\n", bio_state.node_count);

    // 2. Initialize TsfiZmmVmState and Bind Receptors to zmm0..zmm15 Registers
    printf("\n2. Binding Biological Receptors directly into ZMM Hardware Registers...\n");
    TsfiZmmVmState zmm_vm;
    tsfi_zmm_vm_init(&zmm_vm);

    assert(tsfi_mariner_bio_bind_to_zmm(&bio_state, &zmm_vm));

    // Verify ZMM0 and ZMM15 contain non-zero mapped receptor coordinates
    float *zmm0_floats = (float *)&zmm_vm.registers[0];
    float *zmm15_floats = (float *)&zmm_vm.registers[15];

    assert(zmm0_floats[0] != 0.0f || zmm0_floats[1] != 0.0f);
    assert(zmm15_floats[0] != 0.0f || zmm15_floats[1] != 0.0f);

    printf("   ✓ ZMM0 Initial Coordinate Tuple: (x=%.2f, y=%.2f, z=%.2f, charge=%.2f)\n",
           zmm0_floats[0], zmm0_floats[1], zmm0_floats[2], zmm0_floats[3]);
    printf("   ✓ ZMM15 Coordinate Tuple: (x=%.2f, y=%.2f, z=%.2f, charge=%.2f)\n",
           zmm15_floats[0], zmm15_floats[1], zmm15_floats[2], zmm15_floats[3]);

    tsfi_zmm_vm_destroy(&zmm_vm);

    // 3. File and Settle on Chancery Docket
    printf("\n3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_7010 = tsfi_chancery_docket_file(
        &docket,
        "Mariner Phase 2 Biological Target Receptor to ZMM wave512 Register Binding",
        "solidity/dysnomia/domain/std/mariner_biological_receptor_transduction.algol61",
        2026
    );
    assert(doc_7010 == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_7010, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 ZMM REGISTER BINDING TEST PASSED & SEALED ON RECORD\n");
    printf("====================================================================\n");

    return 0;
}
