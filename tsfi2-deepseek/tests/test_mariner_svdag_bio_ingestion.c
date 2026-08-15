#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_svdag.h"
#include "../inc/tsfi_mariner_biological_receptor.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("MARINER PHASE 2: BIOLOGICAL RECEPTOR TO HELMHOLTZ SVDAG INGESTION TEST\n");
    printf("====================================================================\n\n");

    // 1. Ingest Biological Sequence and Transduce 3D Receptors
    MarinerBiologicalState bio_state;
    tsfi_mariner_bio_init(&bio_state);

    const char *sequence = "MEEPQSDPSVEPPLSQETFSDLWKLLPENNVLSPLPSQAMDDLMLSPDDIEQWFTEDPGPDEAPRMPEAAP";
    assert(tsfi_mariner_bio_ingest_sequence(&bio_state, 8003, sequence));
    assert(tsfi_mariner_bio_transduce_wave512_simd(&bio_state, 8));
    assert(bio_state.is_receptor_bound);

    printf("1. Biological Sequence Transduced:\n");
    printf("   Sequence ID:      %u\n", bio_state.sequence_id);
    printf("   Sequence Length:  %lu bases\n", (unsigned long)bio_state.sequence_length);
    printf("   Receptor Nodes:   %u\n", bio_state.node_count);

    // 2. Create Helmholtz SVDAG Taste Tree
    printf("\n2. Creating Helmholtz SVDAG Geometric Taste Tree...\n");
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(1024);
    assert(dag != NULL);

    // Ingest biological receptors into SVDAG voxel streams
    int rc = tsfi_svdag_ingest_biological_receptor(dag, &bio_state);
    assert(rc == 0);
    assert(dag->stream_size == bio_state.node_count);

    printf("   ✓ SVDAG Stream Size: %lu voxels\n", (unsigned long)dag->stream_size);
    printf("   ✓ Voxel 0 Spatial Index: 0x%08X (Intensity: %.2f, Phase: %.2f)\n",
           dag->index_stream[0], dag->intensity_stream[0], dag->phase_stream[0]);
    printf("   ✓ Voxel 70 Spatial Index: 0x%08X (Intensity: %.2f, Phase: %.2f)\n",
           dag->index_stream[dag->stream_size - 1],
           dag->intensity_stream[dag->stream_size - 1],
           dag->phase_stream[dag->stream_size - 1]);

    // 3. File and Resolve on Chancery Docket
    printf("\n3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_7007 = tsfi_chancery_docket_file(
        &docket,
        "Mariner Phase 2 Biological Target Receptor to Helmholtz SVDAG Voxel Ingestion",
        "solidity/dysnomia/domain/std/mariner_biological_receptor_transduction.algol61",
        2026
    );
    assert(doc_7007 == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_7007, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 SVDAG VOXEL INGESTION TEST PASSED & SEALED ON RECORD\n");
    printf("====================================================================\n");

    return 0;
}
