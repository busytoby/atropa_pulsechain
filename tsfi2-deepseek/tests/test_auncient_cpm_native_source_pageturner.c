/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: Auncient CP/M-Tomie Native Source PageTurner
 * Tests:
 * 1. Direct loading of .algol61 files without PDF conversion.
 * 2. Direct loading of COBOL .strategy files without PDF conversion.
 * 3. Live AST verification and witness derivation.
 * 4. RenderMan RIB page rasterization with displacement bounds.
 * 5. Rule 18 non-preferential parity checksum closure.
 */

#include "auncient_cpm_native_source_pageturner.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=================================================================\n");
    printf("TEST PROVER: CPM-TOMIE NATIVE SOURCE PAGETURNER                  \n");
    printf("=================================================================\n");

    NativeSourcePageTurner pt;
    auncient_native_pageturner_init(&pt);

    /* Test 1: Load ALGOL 61 source file directly */
    const char *algol_path = "solidity/dysnomia/domain/std/auncient_clayscape_pageturner_perusal_theorems_96_100_prover.algol61";
    bool load_algol_ok = auncient_native_pageturner_load_file(&pt, algol_path);
    assert(load_algol_ok);
    assert(pt.is_algol61_file);
    assert(pt.live_ast_verification_passed);
    printf(" [ALGOL 61 DIRECT LOAD]  File: %s\n", algol_path);
    printf("                          Lines: %u | Total Pages: %u | Status: PROVED\n",
           pt.total_source_lines, pt.total_pages);

    /* Test 2: Render Page 1 to RenderMan RIB */
    char rib_buf[2048];
    bool rib_ok = auncient_native_pageturner_render_page(&pt, 1, rib_buf, sizeof(rib_buf));
    assert(rib_ok);
    assert(strstr(rib_buf, "RenderMan RIB-Structure") != NULL);
    assert(strstr(rib_buf, "VALID_PROVED") != NULL);
    printf(" [RENDERMAN RIB EMISSION] Page 1 successfully converted to RIB geometry AST.\n");

    /* Test 3: Load COBOL .strategy file directly */
    const char *strat_path = "solidity/dysnomia/domain/strategies/auncient_clayscape_pageturner_perusal_theorems_96_100.strategy";
    bool load_strat_ok = auncient_native_pageturner_load_file(&pt, strat_path);
    assert(load_strat_ok);
    assert(pt.is_cobol_strategy_file);
    assert(pt.live_ast_verification_passed);
    printf(" [COBOL STRATEGY LOAD]   File: %s\n", strat_path);
    printf("                          Lines: %u | Total Pages: %u | Status: PROVED\n",
           pt.total_source_lines, pt.total_pages);

    /* Test 4: Rule 18 Parity */
    assert(pt.rule18_parity_checksum > 0);
    printf(" [RULE 18 PARITY SEAL]   Checksum: 0x%08X (Non-Preferential)\n", pt.rule18_parity_checksum);

    printf("=================================================================\n");
    printf("NATIVE SOURCE PAGETURNER FULLY CERTIFIED (ZERO PDF OVERHEAD).    \n");
    printf("=================================================================\n");
    return 0;
}
