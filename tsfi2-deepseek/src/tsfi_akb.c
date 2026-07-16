#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_akb.h"

TSFiAKB* tsfi_akb_create(tsfi_dat *disk_dat) {
    TSFiAKB *akb = calloc(1, sizeof(TSFiAKB));
    if (!akb) return NULL;
    akb->disk_dat = disk_dat;
    return akb;
}

void tsfi_akb_write(TSFiAKB *akb, const char *key, const char *value) {
    if (!akb || !key || !value) return;
    
    // Check if key is already in cache
    for (int i = 0; i < akb->cache_count; i++) {
        if (strcmp(akb->cache[i].key, key) == 0) {
            strncpy(akb->cache[i].value, value, sizeof(akb->cache[i].value) - 1);
            akb->cache[i].is_dirty = 1;
            return;
        }
    }

    // Insert new entry if room exists
    if (akb->cache_count < MAX_AKB_CACHE_KEYS) {
        int idx = akb->cache_count++;
        strncpy(akb->cache[idx].key, key, sizeof(akb->cache[idx].key) - 1);
        strncpy(akb->cache[idx].value, value, sizeof(akb->cache[idx].value) - 1);
        akb->cache[idx].is_dirty = 1;
    }
}

const char* tsfi_akb_read(TSFiAKB *akb, const char *key) {
    if (!akb || !key) return NULL;

    // 1. Check cache first
    for (int i = 0; i < akb->cache_count; i++) {
        if (strcmp(akb->cache[i].key, key) == 0) {
            return akb->cache[i].value;
        }
    }

    // 2. Fallback to disk DAT rails
    if (akb->disk_dat) {
        return tsfi_dat_search(akb->disk_dat, key);
    }

    return NULL;
}

void tsfi_akb_sync(TSFiAKB *akb, tsfi_trie_node *trie_root) {
    if (!akb || !trie_root) return;

    int synced_any = 0;
    for (int i = 0; i < akb->cache_count; i++) {
        if (akb->cache[i].is_dirty) {
            tsfi_trie_insert(trie_root, akb->cache[i].key, akb->cache[i].value);
            akb->cache[i].is_dirty = 0;
            synced_any = 1;
        }
    }

    if (synced_any && akb->disk_dat) {
        tsfi_dat_destroy(akb->disk_dat);
        akb->disk_dat = tsfi_dat_compile(trie_root);
    }
}

static int get_or_create_node(TSFiAKBGraphIndex *g, const char *name) {
    for (int i = 0; i < g->active_nodes; i++) {
        if (strcmp(g->node_names[i], name) == 0) return i;
    }
    if (g->active_nodes < MAX_ADJACENCY_NODES) {
        int idx = g->active_nodes++;
        strncpy(g->node_names[idx], name, sizeof(g->node_names[idx]) - 1);
        return idx;
    }
    return -1;
}

void tsfi_akb_add_edge(TSFiAKB *akb, const char *src, const char *dest, float weight) {
    if (!akb || !src || !dest) return;
    
    int u = get_or_create_node(&akb->graph, src);
    int v = get_or_create_node(&akb->graph, dest);

    if (u >= 0 && v >= 0) {
        akb->graph.adjacency[u][v] = weight;
    }
}

float tsfi_akb_get_edge(TSFiAKB *akb, const char *src, const char *dest) {
    if (!akb || !src || !dest) return 0.0f;
    
    int u = -1, v = -1;
    for (int i = 0; i < akb->graph.active_nodes; i++) {
        if (strcmp(akb->graph.node_names[i], src) == 0) u = i;
        if (strcmp(akb->graph.node_names[i], dest) == 0) v = i;
    }

    if (u >= 0 && v >= 0) {
        return akb->graph.adjacency[u][v];
    }
    return 0.0f;
}

int tsfi_markov_vm_execute(TSFiMarkovVM *vm, const int *bytecode, size_t len) {
    if (!vm || !bytecode || len == 0) return -1;

    size_t pc = 0;
    vm->halted = 0;

    while (pc < len && !vm->halted) {
        int op = bytecode[pc++];
        switch (op) {
            case 1: // OP_PUSH
                if (pc >= len) return -2;
                vm->stack[vm->stack_len++] = bytecode[pc++];
                break;
            case 6: // OP_HALT
                vm->halted = 1;
                break;
            case 0x31: // OP_WEIGHT: Pops weight value, assigns to next weight register
                if (vm->stack_len < 1) return -4;
                if (vm->weight_count < 16) {
                    vm->weight_registers[vm->weight_count++] = (float)vm->stack[--vm->stack_len] / 100.0f;
                }
                break;
            case 0x32: // OP_SAT: Pops limit weight, returns 1 if total registered weight satisfies limits, else 0
                if (vm->stack_len < 1) return -4;
                {
                    float limit = (float)vm->stack[--vm->stack_len] / 100.0f;
                    float total_weight = 0.0f;
                    for (int i = 0; i < vm->weight_count; i++) {
                        total_weight += vm->weight_registers[i];
                    }
                    vm->stack[vm->stack_len++] = (total_weight >= limit) ? 1 : 0;
                }
                break;
            default:
                return -5; // Unknown opcode
        }
    }

    return 0;
}

void tsfi_akb_destroy(TSFiAKB *akb) {
    if (!akb) return;
    free(akb);
}
