#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_winchester_bridge.h"

int main(void) {
    printf("[Winchester Bridge] Starting WinchesterMQ SCSI handshake bridge tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    TSFiWinchesterBridge *bridge = tsfi_winchester_bridge_create(perf_engine);
    assert(bridge != NULL);

    // 1. Verify mapping frequency 440Hz -> keycode 32 (Key 'D')
    perf_engine->synth->wheels[0].frequency = 440.0f;
    int res = tsfi_winchester_bridge_handshake(bridge);
    printf("  [SCSI Handshake] Status: %d, Keycode Register: %d (Expected 32)\n", res, bridge->registers.keycode_reg);
    fflush(stdout);
    assert(res == 0);
    assert(bridge->registers.keycode_reg == 32);

    // 2. Verify mapping frequency 880Hz -> keycode 30 (Key 'A')
    perf_engine->synth->wheels[0].frequency = 880.0f;
    bridge->registers.keycode_reg = 0;
    res = tsfi_winchester_bridge_handshake(bridge);
    printf("  [SCSI Handshake] Status: %d, Keycode Register: %d (Expected 30)\n", res, bridge->registers.keycode_reg);
    fflush(stdout);
    assert(res == 0);
    assert(bridge->registers.keycode_reg == 30);

    // Cleanup
    tsfi_winchester_bridge_destroy(bridge);
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] WinchesterMQ SCSI handshake bridge verified successfully!\n");
    fflush(stdout);
    return 0;
}
