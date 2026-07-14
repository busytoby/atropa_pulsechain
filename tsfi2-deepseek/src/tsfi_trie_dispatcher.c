#include "tsfi_trie_dispatcher.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

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

static uint32_t parse_ip(const char *ip_str) {
    int a, b, c, d;
    if (sscanf(ip_str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
    }
    return 0;
}

static void ip_to_bin_str(uint32_t ip, int prefix_len, char *out_str) {
    for (int i = 0; i < prefix_len; i++) {
        out_str[i] = ((ip >> (31 - i)) & 1) ? '1' : '0';
    }
    out_str[prefix_len] = '\0';
}

tsfi_trie_node* tsfi_trie_init_cidr_router(void) {
    return tsfi_trie_create_node('\0');
}

void tsfi_trie_add_cidr_route(tsfi_trie_node *router, const char *cidr, const char *as_path) {
    char ip_part[64];
    int prefix_len = 32;
    const char *slash = strchr(cidr, '/');
    if (slash) {
        size_t len = slash - cidr;
        if (len < sizeof(ip_part)) {
            strncpy(ip_part, cidr, len);
            ip_part[len] = '\0';
            prefix_len = atoi(slash + 1);
        }
    } else {
        strncpy(ip_part, cidr, sizeof(ip_part) - 1);
        ip_part[sizeof(ip_part) - 1] = '\0';
    }

    uint32_t ip = parse_ip(ip_part);
    char bin_str[64];
    ip_to_bin_str(ip, prefix_len, bin_str);
    tsfi_trie_insert(router, bin_str, as_path);
}

const char* tsfi_trie_route_ip(tsfi_trie_node *router, const char *ip_str) {
    uint32_t ip = parse_ip(ip_str);
    char bin_str[64];
    ip_to_bin_str(ip, 32, bin_str);
    int matched_len = 0;
    return tsfi_trie_longest_prefix(router, bin_str, &matched_len);
}

tsfi_trie_node* tsfi_trie_init_contract_namespace_router(void) {
    tsfi_trie_node *root = tsfi_trie_create_node('\0');
    if (!root) return NULL;

    tsfi_trie_insert(root, "dynamic_", "dynamic_contract");
    tsfi_trie_insert(root, "sys_", "system_contract");
    tsfi_trie_insert(root, "usr_", "user_contract");

    return root;
}

const char* tsfi_trie_resolve_contract_namespace(tsfi_trie_node *router, const char *contract_address) {
    int matched_len = 0;
    return tsfi_trie_longest_prefix(router, contract_address, &matched_len);
}
