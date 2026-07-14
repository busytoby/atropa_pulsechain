#include "tsfi_trie_dispatcher.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

tsfi_trie_node* tsfi_trie_init_rpc_router(void) {
    tsfi_trie_node *root = tsfi_trie_create_node('\0');
    if (!root) return NULL;

    tsfi_trie_insert(root, "wave512.run", "1");
    tsfi_trie_insert(root, "wave512.reset", "2");
    tsfi_trie_insert(root, "wave512.inspect", "3");
    tsfi_trie_insert(root, "wave512.scramble", "4");
    tsfi_trie_insert(root, "wave512.attach", "5");
    tsfi_trie_insert(root, "genetic.benchmark", "6");
    tsfi_trie_insert(root, "shell.read_file", "10");
    tsfi_trie_insert(root, "genetic.establish_llm", "11");
    tsfi_trie_insert(root, "math.motzkin", "12");
    tsfi_trie_insert(root, "genetic.autonomous_optimize", "19");
    tsfi_trie_insert(root, "manifold.swap_asset", "20");
    tsfi_trie_insert(root, "manifold.set_kernel", "21");
    tsfi_trie_insert(root, "manifold.set_active_mask", "22");
    tsfi_trie_insert(root, "manifold.set_secret", "23");
    tsfi_trie_insert(root, "manifold.dispatch", "24");
    tsfi_trie_insert(root, "manifold.inspect_slots", "25");
    tsfi_trie_insert(root, "manifold.upload_asset", "26");
    tsfi_trie_insert(root, "flow.trigger_choreography", "27");
    tsfi_trie_insert(root, "wave512.dilemma_log", "28");
    tsfi_trie_insert(root, "manifold.load_dna_llm", "41");
    tsfi_trie_insert(root, "manifold.query_llm", "42");
    tsfi_trie_insert(root, "manifold.get_receipt", "43");
    tsfi_trie_insert(root, "manifold.mount_instrument", "44");
    tsfi_trie_insert(root, "manifold.play_bio", "45");

    return root;
}

int tsfi_trie_resolve_rpc(tsfi_trie_node *router, const char *method_name) {
    const char *val = tsfi_trie_lookup(router, method_name);
    if (val) {
        return atoi(val);
    }
    return 0;
}

tsfi_trie_node* tsfi_trie_init_abi_router(void) {
    tsfi_trie_node *root = tsfi_trie_create_node('\0');
    if (!root) return NULL;

    tsfi_trie_insert(root, "18c1ab9a", "predictMelQuantized");
    tsfi_trie_insert(root, "20c4433b", "synthesizeNeuralWav");
    tsfi_trie_insert(root, "5f60cdf4", "registerSpeaker");

    return root;
}

const char* tsfi_trie_resolve_abi(tsfi_trie_node *router, const char *selector) {
    return tsfi_trie_lookup(router, selector);
}

tsfi_trie_node* tsfi_trie_init_scsi_router(void) {
    tsfi_trie_node *root = tsfi_trie_create_node('\0');
    if (!root) return NULL;

    tsfi_trie_insert(root, "SCSI_INQUIRY", "handshake_inquiry");
    tsfi_trie_insert(root, "SCSI_READ", "handshake_read");
    tsfi_trie_insert(root, "SCSI_WRITE", "handshake_write");

    return root;
}

const char* tsfi_trie_resolve_scsi(tsfi_trie_node *router, const char *scsi_cmd) {
    int matched_len = 0;
    return tsfi_trie_longest_prefix(router, scsi_cmd, &matched_len);
}
