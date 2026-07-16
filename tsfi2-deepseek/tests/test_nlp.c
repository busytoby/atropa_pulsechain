#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_nlp.h"

int main(void) {
    printf("[NLP Parser] Starting unified DCG-CLP parser tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    TSFiParsedCommand cmd;

    // 1. Verify successful "play tone 0 0.80" command
    int res = tsfi_nlp_execute_command(perf_engine, "play tone 0 0.80", &cmd);
    printf("  [DCG Play] Status: %d, Type: %d (Expected CMD_PLAY_TONE=0)\n", res, cmd.type);
    fflush(stdout);
    assert(res == 0);
    assert(cmd.type == CMD_PLAY_TONE);
    assert(cmd.target_wheel_idx == 0);
    assert(fabs(cmd.target_amplitude - 0.80f) < 0.0001f);

    // Verify synthesizer amplitude state reflects update
    assert(fabs(perf_engine->synth->wheels[0].amplitude - 0.80f) < 0.0001f);

    // 2. Verify CLP Constraint violation "play tone 0 1.50" (amplitude out of bounds)
    res = tsfi_nlp_execute_command(perf_engine, "play tone 0 1.50", &cmd);
    printf("  [DCG Play Fail] Status: %d (Expected Constraint Violation=-3)\n", res);
    fflush(stdout);
    assert(res == -3);

    // 3. Verify path validation: "verify path Node_A Node_B"
    res = tsfi_nlp_execute_command(perf_engine, "verify path Node_A Node_B", &cmd);
    printf("  [DCG Verify] Status: %d, Type: %d, Src: %s, Dest: %s\n", res, cmd.type, cmd.path_src, cmd.path_dest);
    fflush(stdout);
    assert(res == 0);
    assert(cmd.type == CMD_VERIFY_PATH);
    assert(strcmp(cmd.path_src, "Node_A") == 0);
    assert(strcmp(cmd.path_dest, "Node_B") == 0);

    // Verify adjacency matrix was populated
    float edge_w = tsfi_akb_get_edge(perf_engine->synth->orchestrator->akb, "Node_A", "Node_B");
    assert(fabs(edge_w - 0.95f) < 0.0001f);

    // Cleanup
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Unified DCG-CLP parser verified successfully!\n");
    fflush(stdout);
    return 0;
}
