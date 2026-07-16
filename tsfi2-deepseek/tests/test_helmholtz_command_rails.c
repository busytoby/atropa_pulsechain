#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_winchester_bridge.h"

// Helmholtz dispatch function called from VM contract
int dispatch_helmholtz_command(TSFiAnvilVM *vm, TsfiZmmVmState *zmm, TSFiWinchesterBridge *bridge, int keycode) {
    if (!vm || !zmm || !bridge) return -1;

    // 1. Run local contract state binding (Black Rail logic)
    char coord_name[64];
    snprintf(coord_name, sizeof(coord_name), "svdag/helmholtz/%d", keycode);
    tsfi_anvil_vm_bind(vm, coord_name, "DISPATCHED");

    // 2. Dispatch command to ZMM context
    char transaction[256];
    snprintf(transaction, sizeof(transaction), "CALL helmholtz_command_dispatch execute %d", keycode);
    tsfi_zmm_vm_exec(zmm, transaction);

    // 3. Modulate Tone Wheel frequency directly via Winchester bridge target
    if (keycode == 30) {
        bridge->perf_engine->synth->wheels[0].frequency = 880.0f; // Set to Key A tone
    } else if (keycode == 32) {
        bridge->perf_engine->synth->wheels[0].frequency = 440.0f; // Set to Key D tone
    }

    return 0;
}

int main(void) {
    printf("[Helmholtz Command Rails] Starting Helmholtz command dispatch from Rails VM...\n");
    fflush(stdout);

    // 1. Setup Anvil VM (Rails)
    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    // 2. Setup ZMM VM
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    // 3. Setup Synth and Winchester Bridge
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    TSFiWinchesterBridge *bridge = tsfi_winchester_bridge_create(perf_engine);
    assert(bridge != NULL);

    // Set initial tone-wheel frequency
    perf_engine->synth->wheels[0].frequency = 100.0f;

    // 4. Dispatch Helmholtz command: Keycode 30 (Key A)
    int status = dispatch_helmholtz_command(&vm, &zmm, bridge, 30);
    assert(status == 0);
    assert(vm.trail_len == 1);
    printf("  [Rails Contract] Dispatched Helmholtz command (keycode: 30), new frequency: %.2f Hz (Expected: 880.00 Hz)\n",
           perf_engine->synth->wheels[0].frequency);
    fflush(stdout);
    assert(perf_engine->synth->wheels[0].frequency == 880.0f);

    // Cleanup
    tsfi_winchester_bridge_destroy(bridge);
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Helmholtz command dispatch from Rails VM verified successfully!\n");
    fflush(stdout);
    return 0;
}
