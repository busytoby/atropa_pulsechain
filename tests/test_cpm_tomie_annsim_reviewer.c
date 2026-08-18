/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: CP/M-Tomie ANNSIM SimAUD PDF Multi-Phase Reviewer & Proof Synthesizer
 */

#include "cpm_tomie_annsim_reviewer.h"
#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

int main(void) {
    const char *pdf_path = "/home/mariarahel/Downloads/ANNSIM.2025.SimAUD.58.pdf";

    printf("=================================================================\n");
    printf("TEST PROVER: CP/M-TOMIE ANNSIM SIMAUD PDF MULTI-PHASE REVIEWER\n");
    printf("=================================================================\n");

    CpmTomieAnnSimReviewResult *res = cpm_tomie_review_annsim_pdf(pdf_path);
    assert(res != NULL);

    /* Phase 1 Verification: Ingestion & Pure C Inflate */
    assert(res->phase1_ingestion_valid);
    assert(res->total_streams == 96);
    assert(res->extracted_bytes > 30000);
    printf(" Phase 1 [Ingestion & Decompression]: PASS (Streams: %zu, Bytes: %zu)\n",
           res->total_streams, res->extracted_bytes);

    /* Phase 2 Verification: Structural Section Parsing */
    assert(res->phase2_structural_valid);
    printf(" Phase 2 [Structural Sections]:       PASS (Abstract, Intro, Conclusion Verified)\n");

    /* Phase 3 Verification: Domain Axioms & Mention Metrics */
    assert(res->phase3_domain_axioms_valid);
    assert(res->living_lab_count >= 10);
    assert(res->digital_twin_count >= 10);
    assert(res->bim_count >= 5);
    assert(res->sensor_count >= 5);
    assert(res->bas_count >= 5);
    printf(" Phase 3 [Domain Ontology & Axioms]:  PASS (Living Lab: %zu, Digital Twin: %zu, BAS: %zu)\n",
           res->living_lab_count, res->digital_twin_count, res->bas_count);

    /* Phase 4 Verification: Rule 18 Parity Checksum */
    assert(res->phase4_rule18_valid);
    assert(res->final_checksum == 0x00003E00);
    printf(" Phase 4 [Rule 18 Parity Checksum]:   PASS (0x%08X Verified)\n", res->final_checksum);

    /* Phase 5 Verification: Multi-Phase Formal Closure */
    assert(res->phase5_formal_closure_valid);
    printf(" Phase 5 [Multi-Phase Formal Proof]:  PASS (Soundness Proved)\n");

    cpm_tomie_review_result_free(res);
    printf("=================================================================\n");
    printf("ALL ANNSIM SIMAUD REVIEW PROVER TESTS PASSED SUCCESSFULLY.\n");
    printf("=================================================================\n");
    return 0;
}
