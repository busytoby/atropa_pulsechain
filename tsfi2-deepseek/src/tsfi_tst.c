#include "tsfi_tst.h"
#include <stdlib.h>
#include <string.h>

tsfi_tst_node* tsfi_tst_create_node(char ch) {
    tsfi_tst_node *node = (tsfi_tst_node*)malloc(sizeof(tsfi_tst_node));
    if (node) {
        node->ch = ch;
        node->value = NULL;
        node->left = NULL;
        node->eq = NULL;
        node->right = NULL;
    }
    return node;
}

tsfi_tst_node* tsfi_tst_insert(tsfi_tst_node *node, const char *key, const char *value) {
    if (!key || *key == '\0') return node;

    if (!node) {
        node = tsfi_tst_create_node(*key);
    }

    if (*key < node->ch) {
        node->left = tsfi_tst_insert(node->left, key, value);
    } else if (*key > node->ch) {
        node->right = tsfi_tst_insert(node->right, key, value);
    } else {
        if (*(key + 1) == '\0') {
            if (node->value) {
                free(node->value);
            }
            node->value = strdup(value);
        } else {
            node->eq = tsfi_tst_insert(node->eq, key + 1, value);
        }
    }
    return node;
}

const char* tsfi_tst_search(tsfi_tst_node *node, const char *key) {
    if (!node || !key || *key == '\0') return NULL;

    if (*key < node->ch) {
        return tsfi_tst_search(node->left, key);
    } else if (*key > node->ch) {
        return tsfi_tst_search(node->right, key);
    } else {
        if (*(key + 1) == '\0') {
            return node->value;
        }
        return tsfi_tst_search(node->eq, key + 1);
    }
}

void tsfi_tst_destroy(tsfi_tst_node *node) {
    if (!node) return;

    tsfi_tst_destroy(node->left);
    tsfi_tst_destroy(node->eq);
    tsfi_tst_destroy(node->right);

    if (node->value) {
        free(node->value);
    }
    free(node);
}

tsfi_tst_node* tsfi_tst_init_rpc_router(void) {
    tsfi_tst_node *root = NULL;

    root = tsfi_tst_insert(root, "wave512.run", "1");
    root = tsfi_tst_insert(root, "wave512.reset", "2");
    root = tsfi_tst_insert(root, "wave512.inspect", "3");
    root = tsfi_tst_insert(root, "wave512.scramble", "4");
    root = tsfi_tst_insert(root, "wave512.attach", "5");
    root = tsfi_tst_insert(root, "genetic.benchmark", "6");
    root = tsfi_tst_insert(root, "shell.read_file", "10");
    root = tsfi_tst_insert(root, "genetic.establish_llm", "11");
    root = tsfi_tst_insert(root, "math.motzkin", "12");
    root = tsfi_tst_insert(root, "genetic.autonomous_optimize", "19");
    root = tsfi_tst_insert(root, "manifold.swap_asset", "20");
    root = tsfi_tst_insert(root, "manifold.set_kernel", "21");
    root = tsfi_tst_insert(root, "manifold.set_active_mask", "22");
    root = tsfi_tst_insert(root, "manifold.set_secret", "23");
    root = tsfi_tst_insert(root, "manifold.dispatch", "24");
    root = tsfi_tst_insert(root, "manifold.inspect_slots", "25");
    root = tsfi_tst_insert(root, "manifold.upload_asset", "26");
    root = tsfi_tst_insert(root, "flow.trigger_choreography", "27");
    root = tsfi_tst_insert(root, "wave64.dilemma_log", "28");
    root = tsfi_tst_insert(root, "wave64.telemetry", "29");
    root = tsfi_tst_insert(root, "wave64.inject_event", "33");
    root = tsfi_tst_insert(root, "wave64.clear_cache", "34");
    root = tsfi_tst_insert(root, "wave64.query_knowledge_graph", "35");
    root = tsfi_tst_insert(root, "manifold.load_dna_llm", "41");
    root = tsfi_tst_insert(root, "manifold.query_llm", "42");
    root = tsfi_tst_insert(root, "manifold.get_receipt", "43");
    root = tsfi_tst_insert(root, "manifold.mount_instrument", "44");
    root = tsfi_tst_insert(root, "manifold.play_bio", "45");
    root = tsfi_tst_insert(root, "wave64.get_price_in_pls", "51");
    root = tsfi_tst_insert(root, "wave64.get_all_prices", "52");
    root = tsfi_tst_insert(root, "wave64.get_token_holders", "53");
    root = tsfi_tst_insert(root, "wave64.add_discovered_token", "54");
    root = tsfi_tst_insert(root, "wave64.add_swap_edge", "55");
    root = tsfi_tst_insert(root, "wave64.retrieve", "56");
    root = tsfi_tst_insert(root, "wave64.augment", "57");
    root = tsfi_tst_insert(root, "wave64.prune", "58");
    root = tsfi_tst_insert(root, "wave64.get_unpriced_tokens", "59");
    root = tsfi_tst_insert(root, "pulsechain.get_latest_block", "60");
    root = tsfi_tst_insert(root, "pulsechain.get_known_contracts", "61");
    root = tsfi_tst_insert(root, "pulsechain.prune_block", "62");
    root = tsfi_tst_insert(root, "wave64.get_dexscreener_price", "63");
    root = tsfi_tst_insert(root, "wave64.get_dexscreener_pairs", "64");
    root = tsfi_tst_insert(root, "cics.audit_terminal_session", "70");
    root = tsfi_tst_insert(root, "cics.audit_queue", "71");
    root = tsfi_tst_insert(root, "cics.inject_ballistic_data", "72");
    root = tsfi_tst_insert(root, "cics.audit_web_gateway", "73");
    root = tsfi_tst_insert(root, "cics.audit_micr", "74");
    root = tsfi_tst_insert(root, "cics.audit_atm_transaction", "75");
    root = tsfi_tst_insert(root, "cics.issue_atm_card", "76");
    root = tsfi_tst_insert(root, "cics.omp_feilong_command", "77");
    root = tsfi_tst_insert(root, "cics.omp_galasa_run_suite", "78");

    return root;
}

int tsfi_tst_resolve_rpc(tsfi_tst_node *router, const char *method_name) {
    const char *val = tsfi_tst_search(router, method_name);
    if (val) {
        return atoi(val);
    }
    return 0;
}
