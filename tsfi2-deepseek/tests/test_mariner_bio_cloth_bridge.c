#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_mariner_biological_receptor.h"
#include "../src/auncient_cloth_material_bridge.h"
#include "../src/cloth_simulator.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("MARINER PHASE 2: BIOLOGICAL RECEPTOR TO CLOTH DEFORMATION BRIDGE TEST\n");
    printf("====================================================================\n\n");

    // 1. Ingest Real Biological Sequence
    MarinerBiologicalState bio_state;
    tsfi_mariner_bio_init(&bio_state);

    const char *sequence = "MKWVTFISLLFLFSSAYSRGVFRRDTHKSEIAHRFKDLGEEHFKGLVLIAFSQYLQQCPFDEHVKLVNEL";
    assert(tsfi_mariner_bio_ingest_sequence(&bio_state, 8002, sequence));
    assert(tsfi_mariner_bio_transduce_wave512_simd(&bio_state, 8));
    assert(bio_state.is_receptor_bound);

    printf("1. Biological Sequence Transduced:\n");
    printf("   Length: %lu bases | Receptor Nodes: %u\n",
           (unsigned long)bio_state.sequence_length, bio_state.node_count);

    // 2. Initialize Simulated Cloth Mesh Vertices
    #define TEST_CLOTH_NODES 64
    ClothVertex vertices[TEST_CLOTH_NODES];
    memset(vertices, 0, sizeof(vertices));

    for (int i = 0; i < TEST_CLOTH_NODES; i++) {
        vertices[i].x = (float)(i % 8);
        vertices[i].y = (float)(i / 8);
        vertices[i].z = 0.0f;
        vertices[i].u = (float)(i % 8) / 8.0f;
        vertices[i].v = (float)(i / 8) / 8.0f;
        vertices[i].color = 0xFFFFFFFF;
    }

    // 3. Apply Biological Receptor Transduction to Cloth Deformation
    printf("\n2. Applying Biological Receptor Transduction to Cloth Surface...\n");
    auncient_bridge_bio_receptor_to_cloth(&bio_state, vertices, TEST_CLOTH_NODES);

    // Verify non-zero deformation and biological charge tinting
    assert(vertices[0].color != 0xFFFFFFFF);
    assert(vertices[7].x != (float)(7 % 8));
    printf("   ✓ Cloth Mesh Vertex 0 Color Modulated: 0x%08X\n", vertices[0].color);
    printf("   ✓ Cloth Mesh Vertex 7 Position Shifted: (%.4f, %.4f, %.4f)\n",
           vertices[7].x, vertices[7].y, vertices[7].z);

    // 4. File and Resolve Milestone on Chancery Docket
    printf("\n3. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_7006 = tsfi_chancery_docket_file(
        &docket,
        "Mariner Phase 2 Biological Receptor to Cloth Physics Bridge",
        "solidity/dysnomia/domain/std/mariner_biological_receptor_transduction.algol61",
        2026
    );
    assert(doc_7006 == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_7006, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("MARINER PHASE 2 RECEPTOR CLOTH BRIDGE TEST PASSED & SEALED ON RECORD\n");
    printf("====================================================================\n");

    return 0;
}
