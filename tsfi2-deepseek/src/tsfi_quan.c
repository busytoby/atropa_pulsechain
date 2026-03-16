#include "tsfi_quan.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static QuanNode* create_node() {
    QuanNode *node = (QuanNode*)lau_calloc(1, sizeof(QuanNode));
    return node;
}

static void destroy_node(QuanNode *node) {
    if (!node) return;
    for (int i = 0; i < 128; i++) {
        if (node->children[i]) destroy_node(node->children[i]);
    }
    lau_free(node);
}

QuanState* tsfi_quan_init() {
    QuanState *qs = (QuanState*)lau_calloc(1, sizeof(QuanState));
    qs->root = create_node();
    qs->current = qs->root;
    qs->is_active = false;
    return qs;
}

void tsfi_quan_destroy(QuanState *qs) {
    if (!qs) return;
    destroy_node(qs->root);
    lau_free(qs);
}

void tsfi_quan_add_mapping(QuanState *qs, const char *path, const uint32_t *cluster, int len, uint32_t script_id) {
    QuanNode *curr = qs->root;
    for (int i = 0; path[i]; i++) {
        uint8_t key = (uint8_t)path[i];
        if (key >= 128) continue;
        if (!curr->children[key]) curr->children[key] = create_node();
        curr = curr->children[key];
    }
    
    if (curr->candidate_count < MAX_QUAN_CANDIDATES) {
        QuanCandidate *c = &curr->candidates[curr->candidate_count++];
        memcpy(c->cluster, cluster, len * sizeof(uint32_t));
        c->cluster_len = len;
        c->script_id = script_id;
        strncpy(c->path, path, 31);
    }
}

void tsfi_quan_reset(QuanState *qs) {
    qs->current = qs->root;
    qs->input_len = 0;
    qs->candidate_page = 0;
    memset(qs->input_buffer, 0, 32);
}

bool tsfi_quan_input(QuanState *qs, char c, uint32_t *out_cluster, int *out_len) {
    if (!qs->is_active) return false;

    uint8_t key = (uint8_t)c;
    if (key >= 128) return false;

    // Logic: If SPACE is pressed and we have uncommitted input, we commit the best candidate.
    if (c == ' ' && qs->input_len > 0) {
        if (qs->current->candidate_count > 0) {
            // Find best candidate based on locked script
            int best_idx = 0;
            if (qs->locked_script_id != 0) {
                for (int i=0; i<qs->current->candidate_count; i++) {
                    if (qs->current->candidates[i].script_id == qs->locked_script_id) {
                        best_idx = i;
                        break;
                    }
                }
            }
            QuanCandidate *cand = &qs->current->candidates[best_idx];
            memcpy(out_cluster, cand->cluster, cand->cluster_len * sizeof(uint32_t));
            *out_len = cand->cluster_len;
            tsfi_quan_reset(qs);
            return true;
        }
        tsfi_quan_reset(qs);
        return true;
    }

    // Handle number keys (1-9) for selecting candidates
    if (qs->input_len > 0 && c >= '1' && c <= '9') {
        int selection = c - '1';
        int target_idx = (qs->candidate_page * 9) + selection;
        if (target_idx < qs->current->candidate_count) {
            QuanCandidate *cand = &qs->current->candidates[target_idx];
            memcpy(out_cluster, cand->cluster, cand->cluster_len * sizeof(uint32_t));
            *out_len = cand->cluster_len;
            tsfi_quan_reset(qs);
            return true;
        }
        // Ignore invalid selection
        return true; 
    }

    // Handle paging keys
    if (qs->input_len > 0) {
        if (c == '=') {
            int max_page = (qs->current->candidate_count > 0) ? ((qs->current->candidate_count - 1) / 9) : 0;
            if (qs->candidate_page < max_page) qs->candidate_page++;
            return true;
        }
        if (c == '-') {
            if (qs->candidate_page > 0) qs->candidate_page--;
            return true;
        }
    }

    // Standard character input
    if (qs->current->children[key]) {
        qs->current = qs->current->children[key];
        if (qs->input_len < 31) qs->input_buffer[qs->input_len++] = c;
        qs->candidate_page = 0; // Reset page on new input
        return true;
    }

    // If path is broken but we are active, we might want to reset or stay at root
    // For Pin Lu, we usually reset if the character doesn't fit the phonetic rules
    tsfi_quan_reset(qs);
    return false;
}

bool tsfi_quan_backspace(QuanState *qs) {
    if (!qs->is_active || qs->input_len == 0) return false;
    
    // Reverse walk to find previous node
    qs->input_len--;
    qs->input_buffer[qs->input_len] = 0;
    
    QuanNode *curr = qs->root;
    for (int i=0; i<qs->input_len; i++) {
        curr = curr->children[(uint8_t)qs->input_buffer[i]];
    }
    qs->current = curr;
    return true;
}

void tsfi_quan_activate(QuanState *qs, uint32_t script_id) {
    qs->is_active = !qs->is_active;
    qs->locked_script_id = script_id;
    tsfi_quan_reset(qs);
    tsfi_io_printf(stdout, "[QUAN] Activation: %s | Script Context: 0x%X\n", qs->is_active ? "ON" : "OFF", script_id);
}

void tsfi_quan_load_marchen(QuanState *qs) {
    // Punctuation
    uint32_t head_mark = 0x11C70;
    tsfi_quan_add_mapping(qs, "@", &head_mark, 1, TSFI_SCRIPT_MARCHEN);
    uint32_t shad = 0x11C71;
    tsfi_quan_add_mapping(qs, "/", &shad, 1, TSFI_SCRIPT_MARCHEN);

    // Base Consonants (11C72 - 11C8F)
    const char *consonants[] = {
        "ka", "kha", "ga", "nga", "ca", "cha", "ja", "nya",
        "ta", "tha", "da", "na", "pa", "pha", "ba", "ma",
        "tsa", "tsha", "dza", "wa", "zha", "za", "'a", "ya",
        "ra", "la", "sha", "sa", "ha", "a"
    };
    for (int i=0; i<30; i++) {
        uint32_t cp = 0x11C72 + i;
        tsfi_quan_add_mapping(qs, consonants[i], &cp, 1, TSFI_SCRIPT_MARCHEN);
    }

    // Vowels (Suffixes)
    const char *vowel_keys[] = { "i", "u", "e", "o" };
    uint32_t vowels[] = { 0x11C94, 0x11C95, 0x11C96, 0x11C97 };
    
    for (int i=0; i<30; i++) {
        for (int v=0; v<4; v++) {
            char path[16]; snprintf(path, 15, "%s%s", consonants[i], vowel_keys[v]);
            uint32_t cluster[2] = { (uint32_t)(0x11C72 + i), vowels[v] };
            tsfi_quan_add_mapping(qs, path, cluster, 2, TSFI_SCRIPT_MARCHEN);
        }
    }

    // Subjoined RA example
    for (int i=0; i<30; i++) {
        char path[16]; snprintf(path, 15, "%sra", consonants[i]);
        uint32_t cluster[2] = { (uint32_t)(0x11C72 + i), 0x11CB2 };
        tsfi_quan_add_mapping(qs, path, cluster, 2, TSFI_SCRIPT_MARCHEN);
    }

    // Special Marks
    for (int i=0; i<30; i++) {
        char path_m[16]; snprintf(path_m, 15, "%saM", consonants[i]);
        uint32_t cluster_m[2] = { (uint32_t)(0x11C72 + i), 0x11C92 }; 
        tsfi_quan_add_mapping(qs, path_m, cluster_m, 2, TSFI_SCRIPT_MARCHEN);

        char path_c[16]; snprintf(path_c, 15, "%sa~", consonants[i]);
        uint32_t cluster_c[2] = { (uint32_t)(0x11C72 + i), 0x11C93 }; 
        tsfi_quan_add_mapping(qs, path_c, cluster_c, 2, TSFI_SCRIPT_MARCHEN);
    }

    // Universal Demonstration: 'ka' in Sinhala (0x0D9A)
    uint32_t sinhala_ka = 0x0D9A;
    tsfi_quan_add_mapping(qs, "ka", &sinhala_ka, 1, 0x0D80 /* SINHALA */);

    tsfi_io_printf(stdout, "[QUAN] Marchen mappings loaded into Trie.\n");
}
