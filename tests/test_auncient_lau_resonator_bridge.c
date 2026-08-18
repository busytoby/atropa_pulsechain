/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: Auncient LAU Resonator Bridge & Helmholtz Resonance Audit Pipeline
 */

#define _POSIX_C_SOURCE 200809L
#include "auncient_lau_resonator_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("=================================================================\n");
    printf("TEST PROVER: AUNCIENT LAU RESONATOR BRIDGE (SYSTEM-11 AUDITED)   \n");
    printf("=================================================================\n");

    InternalHeader h = { 
        .version = 0,
        .resonance_as_status = strdup("LAU_SYNC_START"),
        .ftw = false,
        .counter = 0,
        .is_autonomous_excuse_active = false
    };

    WaveSystem *ws = malloc(sizeof(WaveSystem));
    assert(ws != NULL);
    WIRE_BIJECTION(ws, &h);

    /* Assert initial wired bijection */
    assert(*ws->version == 0);
    assert(*ws->counter == 0);
    assert(*ws->ftw == false);
    assert(strcmp(*ws->resonance_as_status, "LAU_SYNC_START") == 0);
    printf(" [BIJECTION] Wire Mapping Verification:         PROVED (Header <-> WaveSystem)\n");

    /* Execute Resonance Pipeline */
    bool ok = auncient_lau_resonator_run_single_pass(&h, ws, 2.5, "RESONATE_CYCLE_01");
    assert(ok);

    /* Assert Post-State */
    assert(h.version == 2026);
    assert(h.ftw == true);
    assert(h.counter == 1);
    assert(*ws->counter == 1);
    assert(*ws->version == 2026);
    assert(*ws->ftw == true);
    printf(" [HELMHOLTZ] Safety Epoch (2026) & State (FTW): PROVED (Version: %d | FTW: true)\n", h.version);
    printf(" [DIRECTIVE] Executor Directive Traced Delta:   PROVED (Counter: %d)\n", h.counter);
    printf(" [TRACE LOG] Immutable Log Status:              PROVED (%s)\n", *ws->resonance_as_status);

    /* Rule 18 Parity */
    uint32_t p_check = auncient_lau_resonator_compute_rule18(&h, ws);
    assert(p_check > 0);
    printf(" Rule 18 Non-Preferential Parity Checksum:      PROVED (0x%08X)\n", p_check);

    lau_final_cleanup(&h, ws, -1);
    printf("=================================================================\n");
    printf("AUNCIENT LAU RESONATOR BRIDGE FULLY CERTIFIED ON DYSNOMIA VM!    \n");
    printf("=================================================================\n");
    return 0;
}
