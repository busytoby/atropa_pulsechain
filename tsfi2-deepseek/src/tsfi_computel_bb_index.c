#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/file.h>
#include <openssl/sha.h>
#include "tsfi_zmm_rpc.h"
#include "lau_yul_thunk.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tsfi_computel_blue_box.h"
    #define GRID_SIZE 80
#define MAX_BGP_PEERS 32
#define MAX_GGUF_ROUTES 64
#define MAX_VALIDATOR_BIDS 16
#include <dirent.h>
#include <sys/stat.h>
#include "tsfi_mainframe_decnet.h"


static RbtNode rbt_node_pool[RBT_MAX_NODES];
uint32_t rbt_node_count = 0;
RbtNode *rbt_root = NULL;
static RbtNode* rbt_alloc_node(uint32_t block_number, struct TwoThreeNode *two_three_node) {
    if (rbt_node_count >= RBT_MAX_NODES) return NULL;
    RbtNode *node = &rbt_node_pool[rbt_node_count++];
    node->block_number = block_number;
    node->two_three_node = two_three_node;
    node->color = RBT_RED;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

static void rbt_left_rotate(RbtNode **root, RbtNode *x) {
    RbtNode *y = x->right;
    x->right = y->left;
    if (y->left != NULL) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void rbt_right_rotate(RbtNode **root, RbtNode *y) {
    RbtNode *x = y->left;
    y->left = x->right;
    if (x->right != NULL) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == NULL) *root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
}

static void rbt_insert_fixup(RbtNode **root, RbtNode *z) {
    while (z != *root && z->parent->color == RBT_RED) {
        if (z->parent == z->parent->parent->left) {
            RbtNode *y = z->parent->parent->right;
            if (y != NULL && y->color == RBT_RED) {
                z->parent->color = RBT_BLACK;
                y->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rbt_left_rotate(root, z);
                }
                z->parent->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                rbt_right_rotate(root, z->parent->parent);
            }
        } else {
            RbtNode *y = z->parent->parent->left;
            if (y != NULL && y->color == RBT_RED) {
                z->parent->color = RBT_BLACK;
                y->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbt_right_rotate(root, z);
                }
                z->parent->color = RBT_BLACK;
                z->parent->parent->color = RBT_RED;
                rbt_left_rotate(root, z->parent->parent);
            }
        }
    }
    (*root)->color = RBT_BLACK;
}

void blue_box_rbt_insert(uint32_t block_number, struct TwoThreeNode *two_three_node) {
    RbtNode *z = rbt_alloc_node(block_number, two_three_node);
    if (!z) return;

    RbtNode *y = NULL;
    RbtNode *x = rbt_root;

    while (x != NULL) {
        y = x;
        if (z->block_number < x->block_number) x = x->left;
        else x = x->right;
    }

    z->parent = y;
    if (y == NULL) {
        rbt_root = z;
    } else if (z->block_number < y->block_number) {
        y->left = z;
    } else {
        y->right = z;
    }

    rbt_insert_fixup(&rbt_root, z);
}

struct TwoThreeNode* blue_box_rbt_lookup(uint32_t block_number) {
    RbtNode *x = rbt_root;
    while (x != NULL) {
        if (block_number == x->block_number) return x->two_three_node;
        if (block_number < x->block_number) x = x->left;
        else x = x->right;
    }
    return NULL;
}



static TwoThreeNode *blue_box_tree_root = NULL;

static void blue_box_sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}

TwoThreeNode* blue_box_create_leaf(uint32_t key1, const char *val1, uint32_t key2, const char *val2) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = val2 ? 2 : 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    if (val2) {
        node->keys[1] = key2;
        strcpy(node->values[1], val2);
    }
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s:%u:%s", 
                       node->keys[0], node->values[0], 
                       val2 ? node->keys[1] : 0, val2 ? node->values[1] : "");
    blue_box_sha256(temp, len, node->node_hash);
    return node;
}

TwoThreeNode* blue_box_create_internal(TwoThreeNode *c1, TwoThreeNode *c2, TwoThreeNode *c3) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = false;
    node->children[0] = c1;
    node->children[1] = c2;
    if (c3) {
        node->children[2] = c3;
        node->num_keys = 2;
        node->keys[0] = c2->keys[0];
        node->keys[1] = c3->keys[0];
        uint8_t concat[TWO_THREE_HASH_SIZE * 3];
        memcpy(concat, c1->node_hash, TWO_THREE_HASH_SIZE);
        memcpy(concat + TWO_THREE_HASH_SIZE, c2->node_hash, TWO_THREE_HASH_SIZE);
        memcpy(concat + TWO_THREE_HASH_SIZE * 2, c3->node_hash, TWO_THREE_HASH_SIZE);
        blue_box_sha256(concat, TWO_THREE_HASH_SIZE * 3, node->node_hash);
    } else {
        node->num_keys = 1;
        node->keys[0] = c2->keys[0];
        uint8_t concat[TWO_THREE_HASH_SIZE * 2];
        memcpy(concat, c1->node_hash, TWO_THREE_HASH_SIZE);
        memcpy(concat + TWO_THREE_HASH_SIZE, c2->node_hash, TWO_THREE_HASH_SIZE);
        blue_box_sha256(concat, TWO_THREE_HASH_SIZE * 2, node->node_hash);
    }
    return node;
}

void blue_box_update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s:%u:%s", 
                           node->keys[0], node->values[0], 
                           node->num_keys == 2 ? node->keys[1] : 0, 
                           node->num_keys == 2 ? node->values[1] : "");
        blue_box_sha256(temp, len, node->node_hash);
    } else {
        if (node->children[2]) {
            uint8_t concat[TWO_THREE_HASH_SIZE * 3];
            memcpy(concat, node->children[0]->node_hash, TWO_THREE_HASH_SIZE);
            memcpy(concat + TWO_THREE_HASH_SIZE, node->children[1]->node_hash, TWO_THREE_HASH_SIZE);
            memcpy(concat + TWO_THREE_HASH_SIZE * 2, node->children[2]->node_hash, TWO_THREE_HASH_SIZE);
            blue_box_sha256(concat, TWO_THREE_HASH_SIZE * 3, node->node_hash);
        } else {
            uint8_t concat[TWO_THREE_HASH_SIZE * 2];
            memcpy(concat, node->children[0]->node_hash, TWO_THREE_HASH_SIZE);
            memcpy(concat + TWO_THREE_HASH_SIZE, node->children[1]->node_hash, TWO_THREE_HASH_SIZE);
            blue_box_sha256(concat, TWO_THREE_HASH_SIZE * 2, node->node_hash);
        }
    }
}

const char* blue_box_retrieve_23_data(TwoThreeNode *node, uint32_t key) {
    if (!node) return NULL;
    if (node->is_leaf) {
        if (node->keys[0] == key) return node->values[0];
        if (node->num_keys == 2 && node->keys[1] == key) return node->values[1];
        return NULL;
    }
    if (key < node->keys[0]) {
        return blue_box_retrieve_23_data(node->children[0], key);
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        return blue_box_retrieve_23_data(node->children[1], key);
    } else {
        return blue_box_retrieve_23_data(node->children[2], key);
    }
}

bool blue_box_store_23_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            blue_box_update_node_hash(node);
            return true;
        }
        if (node->num_keys == 2 && node->keys[1] == key) {
            strcpy(node->values[1], new_value);
            blue_box_update_node_hash(node);
            return true;
        }
        return false;
    }
    bool updated = false;
    if (key < node->keys[0]) {
        updated = blue_box_store_23_data(node->children[0], key, new_value);
    } else if (node->num_keys == 1 || key < node->keys[1]) {
        updated = blue_box_store_23_data(node->children[1], key, new_value);
    } else {
        updated = blue_box_store_23_data(node->children[2], key, new_value);
    }
    if (updated) {
        blue_box_update_node_hash(node);
    }
    return updated;
}

void blue_box_free_23_tree(TwoThreeNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        blue_box_free_23_tree(node->children[0]);
        blue_box_free_23_tree(node->children[1]);
        blue_box_free_23_tree(node->children[2]);
    }
    free(node);
}

static int avl_height(AvlNode *n) {
    return n ? n->height : 0;
}

static int avl_balance(AvlNode *n) {
    return n ? avl_height(n->left) - avl_height(n->right) : 0;
}

static AvlNode* avl_right_rotate(AvlNode *y) {
    AvlNode *x = y->left;
    AvlNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + (avl_height(y->left) > avl_height(y->right) ? avl_height(y->left) : avl_height(y->right));
    x->height = 1 + (avl_height(x->left) > avl_height(x->right) ? avl_height(x->left) : avl_height(x->right));
    return x;
}

static AvlNode* avl_left_rotate(AvlNode *x) {
    AvlNode *y = x->right;
    AvlNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + (avl_height(x->left) > avl_height(x->right) ? avl_height(x->left) : avl_height(x->right));
    y->height = 1 + (avl_height(y->left) > avl_height(y->right) ? avl_height(y->left) : avl_height(y->right));
    return y;
}

AvlNode* avl_insert(AvlNode *node, uint32_t val, uint32_t block_number) {
    if (!node) {
        AvlNode *n = (AvlNode*)calloc(1, sizeof(AvlNode));
        n->val = val;
        n->block_number = block_number;
        n->height = 1;
        return n;
    }
    if (val == node->val) {
        node->block_number = block_number;
        return node;
    }
    if (val < node->val) {
        node->left = avl_insert(node->left, val, block_number);
    } else {
        node->right = avl_insert(node->right, val, block_number);
    }

    node->height = 1 + (avl_height(node->left) > avl_height(node->right) ? avl_height(node->left) : avl_height(node->right));
    int balance = avl_balance(node);

    if (balance > 1 && val < node->left->val) {
        return avl_right_rotate(node);
    }
    if (balance < -1 && val >= node->right->val) {
        return avl_left_rotate(node);
    }
    if (balance > 1 && val >= node->left->val) {
        node->left = avl_left_rotate(node->left);
        return avl_right_rotate(node);
    }
    if (balance < -1 && val < node->right->val) {
        node->right = avl_right_rotate(node->right);
        return avl_left_rotate(node);
    }
    return node;
}

void avl_inorder(AvlNode *root, uint32_t *arr, uint32_t *idx, uint32_t max_len) {
    if (!root || *idx >= max_len) return;
    avl_inorder(root->left, arr, idx, max_len);
    if (*idx < max_len) {
        arr[(*idx)++] = root->block_number;
    }
    avl_inorder(root->right, arr, idx, max_len);
}

void avl_free(AvlNode *root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

AvlNode *centrex_avl = NULL;
void blue_box_bind_23_tree(TwoThreeNode *root);
uint32_t blue_box_centrex_lookup(uint32_t dial_code) {
    AvlNode *x = centrex_avl;
    while (x != NULL) {
        if (dial_code == x->val) return x->block_number;
        if (dial_code < x->val) x = x->left;
        else x = x->right;
    }
    return 0;
}

static void avl_inorder_routes(AvlNode *root, uint32_t *keys, uint32_t *vals, uint32_t *idx, uint32_t max_len) {
    if (!root || *idx >= max_len) return;
    avl_inorder_routes(root->left, keys, vals, idx, max_len);
    if (*idx < max_len) {
        keys[*idx] = root->val;
        vals[*idx] = root->block_number;
        (*idx)++;
    }
    avl_inorder_routes(root->right, keys, vals, idx, max_len);
}

uint32_t blue_box_centrex_get_sorted_routes(uint32_t *keys_out, uint32_t *vals_out, uint32_t max_results) {
    uint32_t idx = 0;
    avl_inorder_routes(centrex_avl, keys_out, vals_out, &idx, max_results);
    return idx;
}

void blue_box_centrex_add_alias(uint32_t alias_code, uint32_t target_trunk) {
    centrex_avl = avl_insert(centrex_avl, alias_code, target_trunk);
}

uint32_t blue_box_centrex_resolve_route(uint32_t dial_code) {
    uint32_t resolved = blue_box_centrex_lookup(dial_code);
    return resolved ? resolved : dial_code;
}

void blue_box_centrex_add_unicode_alias(const char *unicode_name, uint32_t target_trunk) {
    if (!unicode_name) return;
    uint32_t hash = calculate_crc32((const uint8_t *)unicode_name, strlen(unicode_name));
    centrex_avl = avl_insert(centrex_avl, hash, target_trunk);
}

uint32_t blue_box_centrex_resolve_unicode_route(const char *unicode_name) {
    if (!unicode_name) return 0;
    uint32_t current = calculate_crc32((const uint8_t *)unicode_name, strlen(unicode_name));
    for (int depth = 0; depth < 8; depth++) {
        uint32_t resolved = blue_box_centrex_lookup(current);
        if (resolved == 0) return 0;
        if (resolved >= 800 && resolved < 832) {
            return resolved; // found physical target trunk
        }
        current = resolved; // keep resolving next hop
    }
    return 0; // recursion limit reached
}

void blue_box_centrex_add_unicode_forward(const char *src_name, const char *dest_name) {
    if (!src_name || !dest_name) return;
    uint32_t src_hash = calculate_crc32((const uint8_t *)src_name, strlen(src_name));
    uint32_t dest_hash = calculate_crc32((const uint8_t *)dest_name, strlen(dest_name));
    centrex_avl = avl_insert(centrex_avl, src_hash, dest_hash);
}

static void rbt_index_23_node(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        blue_box_rbt_insert(node->keys[0], node);
        if (node->num_keys == 2) {
            blue_box_rbt_insert(node->keys[1], node);
        }
    } else {
        rbt_index_23_node(node->children[0]);
        rbt_index_23_node(node->children[1]);
        if (node->children[2]) {
            rbt_index_23_node(node->children[2]);
        }
    }
}

void blue_box_bind_23_tree(TwoThreeNode *root) {
    blue_box_tree_root = root;
    if (root) {
        memcpy(current_block_state.state_hash, root->node_hash, 32);
        rbt_node_count = 0;
        rbt_root = NULL;
        rbt_index_23_node(root);
    }
}

static bool in_transaction = false;
static BlueBoxBlockState transaction_buffer[RBT_MAX_NODES];
static uint32_t transaction_buffer_count = 0;
void blue_box_begin_transaction() {
    in_transaction = true;
    transaction_buffer_count = 0;
}

bool blue_box_add_to_transaction(const BlueBoxBlockState *state) {
    if (!in_transaction || transaction_buffer_count >= RBT_MAX_NODES || !state) return false;
    transaction_buffer[transaction_buffer_count++] = *state;
    return true;
}

bool blue_box_commit_transaction(const char *filepath) {
    if (!in_transaction || !filepath) return false;

    // Audit gas allowance against database dictionary schema rules before committing transaction
    for (uint32_t idx_aud = 0; idx_aud < transaction_buffer_count; idx_aud++) {
        tsfi_dictionary_constraint rules[1];
        rules[0].column_id = 99;
        rules[0].min_val = 10;
        rules[0].max_val = 9999999;
        if (tsfi_audit_constraint(rules, 1, 99, transaction_buffer[idx_aud].gas_allowance) != 0) {
            blue_box_rollback_transaction();
            return false;
        }
    }


    // 1. Write transactional block states to WAL first
    char wal_path[512];
    snprintf(wal_path, sizeof(wal_path), "%s.wal", filepath);
    FILE *wf = fopen(wal_path, "wb");
    if (wf) {
        flock(fileno(wf), LOCK_EX);
        for (uint32_t i = 0; i < transaction_buffer_count; i++) {
            BlueBoxBlockState *state = &transaction_buffer[i];
            state->checksum = calculate_crc32((const uint8_t *)state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
            fwrite(state, sizeof(BlueBoxBlockState), 1, wf);
        }
        fclose(wf);
    }

    // 2. Write to main history ledger
    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "ab");
    if (!hf) return false;

    flock(fileno(hf), LOCK_EX);

    for (uint32_t i = 0; i < transaction_buffer_count; i++) {
        BlueBoxBlockState *state = &transaction_buffer[i];
        fwrite(state, sizeof(BlueBoxBlockState), 1, hf);

        char payload[128];
        snprintf(payload, sizeof(payload), "nonce:%u,gas:%u,unicode:%s,synth:%.2f", state->nonce, state->gas_allowance, state->unicode_desc, state->synth_frequency);
        TwoThreeNode *tt_node = blue_box_create_leaf(state->block_number, payload, 0, NULL);
        blue_box_rbt_insert(state->block_number, tt_node);
    }

    fclose(hf);

    // 3. Commit complete, release/delete WAL log
    remove(wal_path);

    in_transaction = false;
    transaction_buffer_count = 0;
    return true;
}

void blue_box_rollback_transaction() {
    in_transaction = false;
    transaction_buffer_count = 0;
}

