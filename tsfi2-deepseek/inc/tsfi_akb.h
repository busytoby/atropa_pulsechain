#ifndef TSFI_AKB_H
#define TSFI_AKB_H

#include "tsfi_dat.h"

#define MAX_AKB_CACHE_KEYS 128
#define MAX_ADJACENCY_NODES 16

// In-Memory Cache Entry (Dirty-Page Buffer)
typedef struct {
    char key[128];
    char value[128];
    int is_dirty;
} TSFiAKBCacheEntry;

// Secondary Graph Index (Adjacency Matrix)
typedef struct {
    int active_nodes;
    char node_names[MAX_ADJACENCY_NODES][128];
    float adjacency[MAX_ADJACENCY_NODES][MAX_ADJACENCY_NODES];
} TSFiAKBGraphIndex;

// Advanced Knowledge Base Structure
typedef struct {
    tsfi_dat *disk_dat;
    TSFiAKBCacheEntry cache[MAX_AKB_CACHE_KEYS];
    int cache_count;
    TSFiAKBGraphIndex graph;
} TSFiAKB;

// Create and initialize the Advanced Knowledge Base
TSFiAKB* tsfi_akb_create(tsfi_dat *disk_dat);

// In-Memory Fast Write (Dirty-Page write)
void tsfi_akb_write(TSFiAKB *akb, const char *key, const char *value);

// Ephemeral Temporary State Write (Logically deduced temporary coordinates)
void tsfi_akb_write_temp(TSFiAKB *akb, const char *key, const char *value);

// Check if coordinate is a temporary state
int tsfi_akb_is_temp(TSFiAKB *akb, const char *key);

// In-Memory Fast Read (Cache-First read)
const char* tsfi_akb_read(TSFiAKB *akb, const char *key);

// Sync dirty pages to disk DAT rails
void tsfi_akb_sync(TSFiAKB *akb, tsfi_trie_node *trie_root);

// Add graph edge to secondary index
void tsfi_akb_add_edge(TSFiAKB *akb, const char *src, const char *dest, float weight);

// Check if path exists in secondary graph index: returns weight or 0.0f
float tsfi_akb_get_edge(TSFiAKB *akb, const char *src, const char *dest);

// Markov Logic VM structure with Weight registers
typedef struct {
    int stack[64];
    int stack_len;
    float weight_registers[16];
    int weight_count;
    int halted;
} TSFiMarkovVM;

// Execute Markov Logic VM program supporting OP_WEIGHT (0x31) and OP_SAT (0x32)
int tsfi_markov_vm_execute(TSFiMarkovVM *vm, const int *bytecode, size_t len);

// Destroy AKB structures
void tsfi_akb_destroy(TSFiAKB *akb);

#endif // TSFI_AKB_H
