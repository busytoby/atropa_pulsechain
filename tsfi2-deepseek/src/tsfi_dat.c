#include "tsfi_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DAT_INITIAL_CAPACITY 8192

static void resize_dat(tsfi_dat *dat, int min_capacity) {
    int old_cap = dat->capacity;
    int new_cap = old_cap;
    while (new_cap < min_capacity) {
        new_cap *= 2;
    }
    
    dat->base = (int*)realloc(dat->base, new_cap * sizeof(int));
    dat->check = (int*)realloc(dat->check, new_cap * sizeof(int));
    dat->values = (char**)realloc(dat->values, new_cap * sizeof(char*));
    
    for (int i = old_cap; i < new_cap; i++) {
        dat->base[i] = 0;
        dat->check[i] = -1;
        dat->values[i] = NULL;
    }
    dat->capacity = new_cap;
}

// Helper to find a suitable base value for a set of children character branches
static int find_base(tsfi_dat *dat, int *children, int child_count) {
    if (child_count == 0) return 1;
    
    int b = 1;
    while (true) {
        bool ok = true;
        for (int i = 0; i < child_count; i++) {
            int target = b + children[i];
            if (target >= dat->capacity) {
                resize_dat(dat, target + 1);
            }
            if (dat->check[target] != -1) {
                ok = false;
                break;
            }
        }
        if (ok) return b;
        b++;
    }
}

// Recursive function to map standard Trie nodes into base/check arrays
static void dat_compile_node(tsfi_dat *dat, tsfi_trie_node *trie_node, int dat_state) {
    if (!trie_node) return;

    // Collect all children transition characters
    int children_chars[256];
    tsfi_trie_node *children_nodes[256];
    int child_count = 0;

    tsfi_trie_node *child = trie_node->child;
    while (child && child_count < 256) {
        children_chars[child_count] = (unsigned char)child->ch;
        children_nodes[child_count] = child;
        child_count++;
        child = child->sibling;
    }

    if (child_count > 0) {
        // Find a valid base offset
        int b = find_base(dat, children_chars, child_count);
        dat->base[dat_state] = b;

        // Set check values to reserve slots
        for (int i = 0; i < child_count; i++) {
            int next_state = b + children_chars[i];
            dat->check[next_state] = dat_state;
            if (children_nodes[i]->is_end && children_nodes[i]->phoneme) {
                dat->values[next_state] = strdup(children_nodes[i]->phoneme);
            }
        }

        // Recursively compile children
        for (int i = 0; i < child_count; i++) {
            int next_state = b + children_chars[i];
            dat_compile_node(dat, children_nodes[i], next_state);
        }
    }
}

tsfi_dat* tsfi_dat_compile(tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;

    tsfi_dat *dat = (tsfi_dat*)malloc(sizeof(tsfi_dat));
    if (!dat) return NULL;

    dat->capacity = DAT_INITIAL_CAPACITY;
    dat->base = (int*)malloc(dat->capacity * sizeof(int));
    dat->check = (int*)malloc(dat->capacity * sizeof(int));
    dat->values = (char**)malloc(dat->capacity * sizeof(char*));

    for (int i = 0; i < dat->capacity; i++) {
        dat->base[i] = 0;
        dat->check[i] = -1;
        dat->values[i] = NULL;
    }

    // Set root node (state 0)
    dat->check[0] = 0;

    // Compile from root (state 0)
    dat_compile_node(dat, trie_root, 0);

    return dat;
}

const char* tsfi_dat_search(tsfi_dat *dat, const char *key) {
    if (!dat || !key || *key == '\0') return NULL;

    int state = 0;
    while (*key != '\0') {
        int b = dat->base[state];
        if (b == 0) return NULL; // No outbound transitions

        int next = b + (unsigned char)*key;
        if (next >= dat->capacity || dat->check[next] != state) {
            return NULL; // Transition collision or out of bounds
        }
        state = next;
        key++;
    }

    return dat->values[state];
}

void tsfi_dat_destroy(tsfi_dat *dat) {
    if (!dat) return;

    for (int i = 0; i < dat->capacity; i++) {
        if (dat->values[i]) {
            free(dat->values[i]);
        }
    }
    free(dat->base);
    free(dat->check);
    free(dat->values);
    free(dat);
}

tsfi_dat* tsfi_dat_init_ligature_router(void) {
    tsfi_trie_node *temp = tsfi_trie_create_node('\0');
    if (!temp) return NULL;

    tsfi_trie_insert(temp, "fi", "101");
    tsfi_trie_insert(temp, "fl", "102");
    tsfi_trie_insert(temp, "ffi", "103");
    tsfi_trie_insert(temp, "ffl", "104");

    tsfi_dat *dat = tsfi_dat_compile(temp);
    tsfi_trie_destroy(temp);
    return dat;
}

int tsfi_dat_resolve_ligature(tsfi_dat *router, const char *sequence) {
    const char *val = tsfi_dat_search(router, sequence);
    if (val) {
        return atoi(val);
    }
    return 0;
}

tsfi_dat* tsfi_dat_init_scsi_router(void) {
    tsfi_trie_node *temp = tsfi_trie_create_node('\0');
    if (!temp) return NULL;

    tsfi_trie_insert(temp, "SCSI_INQUIRY", "handshake_inquiry");
    tsfi_trie_insert(temp, "SCSI_READ", "handshake_read");
    tsfi_trie_insert(temp, "SCSI_WRITE", "handshake_write");

    tsfi_dat *dat = tsfi_dat_compile(temp);
    tsfi_trie_destroy(temp);
    return dat;
}

const char* tsfi_dat_resolve_scsi(tsfi_dat *router, const char *scsi_cmd) {
    if (!router || !scsi_cmd) return NULL;
    int state = 0;
    const char *last_val = NULL;
    while (*scsi_cmd != '\0') {
        int b = router->base[state];
        if (b == 0) break;
        int next = b + (unsigned char)*scsi_cmd;
        if (next >= router->capacity || router->check[next] != state) {
            break;
        }
        state = next;
        if (router->values[state]) {
            last_val = router->values[state];
        }
        scsi_cmd++;
    }
    return last_val;
}

int tsfi_dat_save_bin(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    // Header
    fwrite("TDAT", 1, 4, fp);
    fwrite(&(dat->capacity), sizeof(int), 1, fp);

    // Arrays
    fwrite(dat->base, sizeof(int), dat->capacity, fp);
    fwrite(dat->check, sizeof(int), dat->capacity, fp);

    // Values (payload strings)
    for (int i = 0; i < dat->capacity; i++) {
        if (dat->values[i]) {
            uint8_t exists = 1;
            fwrite(&exists, 1, 1, fp);
            uint16_t len = (uint16_t)strlen(dat->values[i]);
            fwrite(&len, sizeof(uint16_t), 1, fp);
            fwrite(dat->values[i], 1, len, fp);
        } else {
            uint8_t exists = 0;
            fwrite(&exists, 1, 1, fp);
        }
    }

    fclose(fp);
    return 0;
}

tsfi_dat* tsfi_dat_load_bin(const char *filepath) {
    if (!filepath) return NULL;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return NULL;

    char magic[4];
    if (fread(magic, 1, 4, fp) != 4 || strncmp(magic, "TDAT", 4) != 0) {
        fclose(fp);
        return NULL;
    }

    tsfi_dat *dat = (tsfi_dat*)malloc(sizeof(tsfi_dat));
    if (!dat) {
        fclose(fp);
        return NULL;
    }

    if (fread(&(dat->capacity), sizeof(int), 1, fp) != 1) {
        free(dat);
        fclose(fp);
        return NULL;
    }

    dat->base = (int*)malloc(dat->capacity * sizeof(int));
    dat->check = (int*)malloc(dat->capacity * sizeof(int));
    dat->values = (char**)malloc(dat->capacity * sizeof(char*));

    if (fread(dat->base, sizeof(int), dat->capacity, fp) != (size_t)dat->capacity ||
        fread(dat->check, sizeof(int), dat->capacity, fp) != (size_t)dat->capacity) {
        free(dat->base);
        free(dat->check);
        free(dat->values);
        free(dat);
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < dat->capacity; i++) {
        uint8_t exists;
        if (fread(&exists, 1, 1, fp) != 1) {
            exists = 0;
        }
        if (exists) {
            uint16_t len;
            if (fread(&len, sizeof(uint16_t), 1, fp) != 1) {
                dat->values[i] = NULL;
                continue;
            }
            dat->values[i] = (char*)malloc(len + 1);
            if (fread(dat->values[i], 1, len, fp) != len) {
                // error recovery
            }
            dat->values[i][len] = '\0';
        } else {
            dat->values[i] = NULL;
        }
    }

    fclose(fp);
    return dat;
}
