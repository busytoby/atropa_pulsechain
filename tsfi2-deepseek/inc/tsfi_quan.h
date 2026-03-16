#ifndef TSFI_QUAN_H
#define TSFI_QUAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_SCRIPT_MARCHEN 0x11C70
#define MAX_QUAN_CANDIDATES 32
#define MAX_QUAN_CLUSTER 8

typedef struct {
    uint32_t cluster[MAX_QUAN_CLUSTER];
    uint32_t cluster_len;
    uint32_t script_id;
    char path[32]; // For reverse lookup/feedback
} QuanCandidate;

typedef struct QuanNode {
    QuanCandidate candidates[MAX_QUAN_CANDIDATES];
    int candidate_count;
    struct QuanNode *children[128]; // Full ASCII range
} QuanNode;

typedef struct {
    char input_buffer[32];
    int input_len;
    QuanNode *root;
    QuanNode *current;
    
    int candidate_page; // Current page of candidates (0-based)
    uint32_t locked_script_id; // For CTRL+F context
    bool is_active;
} QuanState;

// API
QuanState* tsfi_quan_init();
void tsfi_quan_destroy(QuanState *qs);

// Configuration
void tsfi_quan_add_mapping(QuanState *qs, const char *path, const uint32_t *cluster, int len, uint32_t script_id);

// Input
// Returns true if Quan consumes the key
bool tsfi_quan_input(QuanState *qs, char c, uint32_t *out_cluster, int *out_len);
bool tsfi_quan_backspace(QuanState *qs);
void tsfi_quan_reset(QuanState *qs);

// Context
void tsfi_quan_activate(QuanState *qs, uint32_t script_id);

// Script Loaders
void tsfi_quan_load_marchen(QuanState *qs);

#endif
