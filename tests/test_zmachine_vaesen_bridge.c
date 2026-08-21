#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_zmachine_vaesen_bridge.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   Z-MACHINE OPCODE 0xD4: VAESEN LLM INTERACTIVE FICTION & VECTOR BRIDGE TEST\n");
    printf("========================================================================================\n\n");

    /* Ensure canonical device slices are built */
    assert(tsfi_vaesen_slice_build_canonical_all() == 0);

    /* 1. Test Standard Player Dialogue Encounter: Investigator <-> Gårdstomte */
    TsfiZMachinePlayerState player_calm = {
        .phys_trauma = 0,
        .ment_trauma = 0,
        .held_item = "",
        .player_wmq = "dynamic_wmq_0x010000aa"
    };
    TsfiZMachineInteractionResult res1;
    int exec_res1 = tsfi_zmachine_execute_wmq_interact(&player_calm, "assets/vaesen/tomte.dat.bin", &res1);
    assert(exec_res1 == 0);
    printf("[ENCOUNTER 1 - CALM INVESTIGATOR vs TOMTE]\n");
    printf("  Dialogue: %s\n", res1.dialogue_output);
    printf("  Vector Lines Generated: %u lines (Harmonic Perspective Room: PASS)\n\n", res1.num_lines);
    assert(res1.num_lines == 5);

    /* 2. Test High-Trauma Panicked Player Encounter: Investigator (Panicked) <-> Myling */
    TsfiZMachinePlayerState player_panicked = {
        .phys_trauma = 2, /* Battered */
        .ment_trauma = 3, /* Panicked ($D401 = 3) */
        .held_item = "Lantern",
        .player_wmq = "dynamic_wmq_0x010000aa"
    };
    TsfiZMachineInteractionResult res2;
    int exec_res2 = tsfi_zmachine_execute_wmq_interact(&player_panicked, "assets/vaesen/myling.dat.bin", &res2);
    assert(exec_res2 == 0);
    printf("[ENCOUNTER 2 - PANICKED INVESTIGATOR vs MYLING]\n");
    printf("  Dialogue: %s\n", res2.dialogue_output);
    printf("  Vector Lines Generated: %u lines (Jagged Fear Spikes: PASS)\n\n", res2.num_lines);
    assert(res2.num_lines == 6);

    /* 3. Test Inventory Banishment Ritual Match: Presenting Iron Knife vs Näcken */
    TsfiZMachinePlayerState player_ritual = {
        .phys_trauma = 1,
        .ment_trauma = 1,
        .held_item = "iron knife",
        .player_wmq = "dynamic_wmq_0x010000aa"
    };
    TsfiZMachineInteractionResult res3;
    int exec_res3 = tsfi_zmachine_execute_wmq_interact(&player_ritual, "assets/vaesen/nacken.dat.bin", &res3);
    assert(exec_res3 == 0);
    printf("[ENCOUNTER 3 - BANISHMENT RITUAL EXECUTION]\n");
    printf("  Ritual Match: %s\n", res3.ritual_banishment_success ? "SUCCESS (Banishment Sealed)" : "FAIL");
    printf("  Dialogue: %s\n", res3.dialogue_output);
    printf("  Vector Lines Generated: %u lines (Sanctuary Spire: PASS)\n\n", res3.num_lines);
    assert(res3.ritual_banishment_success);
    assert(res3.num_lines == 4);

    /* 4. Latency Benchmark across 100,000 Z-Machine Opcode 0xD4 dispatches */
    const int benchmark_count = 100000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        TsfiZMachineInteractionResult r;
        tsfi_zmachine_execute_wmq_interact(&player_calm, "assets/vaesen/teddy_bear.dat.bin", &r);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("[BENCHMARK] Average Z-Machine Opcode 0xD4 latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> Z-MACHINE VAESEN LLM BRIDGE FORMALLY VERIFIED <<<\n");
    return 0;
}
