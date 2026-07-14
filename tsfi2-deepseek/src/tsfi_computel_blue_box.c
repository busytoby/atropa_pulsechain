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

/*
 * Auncient Computel Single-Frequency (SF) & Multi-Frequency (MF) Switch Controller
 * 
 * Simulates trunk seizure via 2600 Hz SF tones and routing via MF digits (Blue Box).
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// MF frequency pairs for trunk dialing
static const float mf_freqs_f1[12] = {700.0f, 700.0f, 900.0f, 700.0f, 900.0f, 1100.0f, 700.0f, 900.0f, 1100.0f, 1300.0f, 1100.0f, 1500.0f};
static const float mf_freqs_f2[12] = {900.0f, 1100.0f, 1100.0f, 1300.0f, 1300.0f, 1300.0f, 1500.0f, 1500.0f, 1500.0f, 1500.0f, 1700.0f, 1700.0f};
// Map characters: '1'-'9', '0', 'K' (KP), 'S' (ST)
static const char mf_char_map[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'K', 'S'};

struct TwoThreeNode;

#define RBT_MAX_NODES 256

typedef enum { RBT_RED, RBT_BLACK } RbtColor;

typedef struct RbtNode {
    uint32_t block_number;
    struct TwoThreeNode *two_three_node;
    RbtColor color;
    struct RbtNode *left;
    struct RbtNode *right;
    struct RbtNode *parent;
} RbtNode;

static RbtNode rbt_node_pool[RBT_MAX_NODES];
static uint32_t rbt_node_count = 0;
static RbtNode *rbt_root = NULL;

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

#define TWO_THREE_HASH_SIZE 32

typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128]; // basic table data
    uint8_t node_hash[TWO_THREE_HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

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

typedef struct AvlNode {
    uint32_t val;
    uint32_t block_number;
    int height;
    struct AvlNode *left;
    struct AvlNode *right;
} AvlNode;

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

static AvlNode* avl_insert(AvlNode *node, uint32_t val, uint32_t block_number) {
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

static void avl_inorder(AvlNode *root, uint32_t *arr, uint32_t *idx, uint32_t max_len) {
    if (!root || *idx >= max_len) return;
    avl_inorder(root->left, arr, idx, max_len);
    if (*idx < max_len) {
        arr[(*idx)++] = root->block_number;
    }
    avl_inorder(root->right, arr, idx, max_len);
}

static void avl_free(AvlNode *root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

static AvlNode *centrex_avl = NULL;

void blue_box_bind_23_tree(TwoThreeNode *root);

typedef struct {
    uint32_t trunk_id;
    const char *address;
} ImmutableBlueBoxStorageEntry;

static const ImmutableBlueBoxStorageEntry immutable_storage[] = {
    {800, "dynamic_0x0000000000000000000000000000000000000800"},
    {801, "dynamic_0x0000000000000000000000000000000000000801"},
    {805, "dynamic_0x0000000000000000000000000000000000000805"},
    {808, "dynamic_0x0000000000000000000000000000000000000808"},
    {815, "dynamic_0x0000000000000000000000000000000000000815"}
};

const char *blue_box_get_immutable_address(uint32_t trunk_id) {
    int entries = sizeof(immutable_storage) / sizeof(immutable_storage[0]);
    for (int i = 0; i < entries; i++) {
        if (immutable_storage[i].trunk_id == trunk_id) {
            return immutable_storage[i].address;
        }
    }
    return NULL;
}

static uint32_t calculate_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

#pragma pack(push, 1)
typedef struct {
    uint32_t block_number;
    uint8_t state_hash[32];
    uint32_t active_trunk_mask;
    uint32_t nonce;
    uint64_t session_key;
    uint32_t gas_allowance;
    bool is_committed;
    uint32_t checksum;
} BlueBoxBlockState;
#pragma pack(pop)

static BlueBoxBlockState current_block_state = {0, {0}, 0, 0, 0, 0, false, 0};

void blue_box_init_block(uint32_t block_number, const uint8_t *initial_hash) {
    current_block_state.block_number = block_number;
    current_block_state.active_trunk_mask = 0;
    current_block_state.nonce = 0;
    current_block_state.session_key = 0xDEADC0DE95346795ULL;
    current_block_state.gas_allowance = 500000;
    current_block_state.is_committed = false;
    current_block_state.checksum = 0;
    if (initial_hash) {
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] = initial_hash[i];
        }
    } else {
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] = 0;
        }
    }
}

void blue_box_register_block_trunk(uint32_t trunk_id) {
    if (trunk_id >= 800 && trunk_id < 832) {
        current_block_state.active_trunk_mask |= (1U << (trunk_id - 800));
    }
}

bool blue_box_commit_block(void) {
    if (current_block_state.is_committed) return false;
    for (int i = 0; i < 32; i++) {
        current_block_state.state_hash[i] ^= (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
    }
    current_block_state.nonce++;
    // Shared session keys rotate using an LCG multiplier sequence
    current_block_state.session_key = (current_block_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
    current_block_state.is_committed = true;
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    return true;
}

static uint64_t MotzkinPrime = 953467954114363ULL;
static uint64_t accumulator_register = 0;

void blue_box_accumulate_state(uint64_t input_signal) {
    accumulator_register = (accumulator_register + input_signal) % MotzkinPrime;
    for (int i = 0; i < 8; i++) {
        current_block_state.state_hash[i] ^= (uint8_t)(accumulator_register >> (i * 8));
    }
}

uint64_t blue_box_get_accumulator(void) {
    return accumulator_register;
}

BlueBoxBlockState blue_box_get_block_state(void) {
    return current_block_state;
}

/* Generates 2600 Hz SF tone to seize simulated trunk line */
void generate_sf_seizure(float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)sin(2.0 * M_PI * 2600.0 * t);
    }
}

/* Generates MF tones to route calls inside tandem trunk switch matrices */
bool generate_mf_tone(char digit, float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return false;

    int idx = -1;
    for (int i = 0; i < 12; i++) {
        if (mf_char_map[i] == digit) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    float f1 = mf_freqs_f1[idx];
    float f2 = mf_freqs_f2[idx];

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)((sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t)) * 0.5);
    }

    return true;
}

/* Generates Red Box payphone coin tones (1700 Hz + 2200 Hz beeps) based on denomination:
   5 (nickel: 1 beep), 10 (dime: 2 beeps), 25 (quarter: 5 beeps).
   Returns the number of samples populated. */
int generate_red_box_coin_tone(int denomination, float *buffer, int max_samples) {
    if (!buffer || max_samples <= 0) return 0;

    int beeps = 0;
    if (denomination == 5) beeps = 1;
    else if (denomination == 10) beeps = 2;
    else if (denomination == 25) beeps = 5;
    else return 0;

    int beep_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms beep
    int pause_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms pause
    int total_samples = beeps * (beep_samples + pause_samples);

    if (total_samples > max_samples) return 0;

    int offset = 0;
    for (int b = 0; b < beeps; b++) {
        // Generate Beep
        for (int i = 0; i < beep_samples; i++) {
            double t = (double)i / SAMPLE_RATE;
            buffer[offset + i] = (float)((sin(2.0 * M_PI * 1700.0 * t) + sin(2.0 * M_PI * 2200.0 * t)) * 0.5);
        }
        offset += beep_samples;

        // Generate Pause
        for (int i = 0; i < pause_samples; i++) {
            buffer[offset + i] = 0.0f;
        }
        offset += pause_samples;
    }

    return offset;
}

static void append_history_record(const char *filepath, const BlueBoxBlockState *state) {
    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "ab");
    if (hf) {
        flock(fileno(hf), LOCK_EX);
        fwrite(state, sizeof(BlueBoxBlockState), 1, hf);
        fclose(hf);
    }
}

bool blue_box_save_state_to_disk(const char *filepath) {
    if (!filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    flock(fileno(f), LOCK_EX);
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);
    return written == 1;
}

static void blue_box_recover_wal(const char *filepath) {
    char wal_path[512];
    snprintf(wal_path, sizeof(wal_path), "%s.wal", filepath);
    FILE *wf = fopen(wal_path, "rb");
    if (!wf) return;

    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "ab");
    if (hf) {
        flock(fileno(hf), LOCK_EX);
        flock(fileno(wf), LOCK_SH);
        BlueBoxBlockState state;
        while (fread(&state, sizeof(BlueBoxBlockState), 1, wf) == 1) {
            uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
            if (calc == state.checksum) {
                fwrite(&state, sizeof(BlueBoxBlockState), 1, hf);
            }
        }
        fclose(hf);
    }
    fclose(wf);
    remove(wal_path);
}

bool blue_box_load_state_from_disk(const char *filepath) {
    if (!filepath) return false;
    blue_box_recover_wal(filepath);
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    flock(fileno(f), LOCK_SH);
    size_t read = fread(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);
    if (read != 1) return false;
    uint32_t calc = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    if (calc != current_block_state.checksum) return false;

    // Rebuild the Red-Black Tree index from history ledger on launch
    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "rb");
    if (hf) {
        flock(fileno(hf), LOCK_SH);
        BlueBoxBlockState hist_state;
        rbt_node_count = 0;
        rbt_root = NULL;
        while (fread(&hist_state, sizeof(BlueBoxBlockState), 1, hf) == 1) {
            uint32_t hist_calc = calculate_crc32((const uint8_t *)&hist_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
            if (hist_calc == hist_state.checksum) {
                char payload[128];
                snprintf(payload, sizeof(payload), "nonce:%u,gas:%u", hist_state.nonce, hist_state.gas_allowance);
                TwoThreeNode *tt_node = blue_box_create_leaf(hist_state.block_number, payload, 0, NULL);
                blue_box_rbt_insert(hist_state.block_number, tt_node);
            }
        }
        fclose(hf);
    }
    return true;
}

bool blue_box_commit_and_persist_with_guard(const char *filepath, uint32_t expected_parent_block, const uint8_t *expected_parent_hash) {
    if (!filepath || !expected_parent_hash) return false;

    FILE *f = fopen(filepath, "r+b");
    if (!f) {
        f = fopen(filepath, "wb");
        if (!f) return false;
        flock(fileno(f), LOCK_EX);
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] ^= (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
        }
        current_block_state.nonce++;
        current_block_state.session_key = (current_block_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
        current_block_state.is_committed = true;
        current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
        fclose(f);
        if (written == 1) {
            append_history_record(filepath, &current_block_state);
            char payload[128];
            snprintf(payload, sizeof(payload), "nonce:%u,gas:%u", current_block_state.nonce, current_block_state.gas_allowance);
            TwoThreeNode *tt_node = blue_box_create_leaf(current_block_state.block_number, payload, 0, NULL);
            blue_box_rbt_insert(current_block_state.block_number, tt_node);
            return true;
        }
        return false;
    }

    flock(fileno(f), LOCK_EX);

    BlueBoxBlockState disk_state;
    if (fread(&disk_state, sizeof(BlueBoxBlockState), 1, f) != 1) {
        fclose(f);
        return false;
    }

    uint32_t disk_calc = calculate_crc32((const uint8_t *)&disk_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    if (disk_calc != disk_state.checksum) {
        fclose(f);
        return false; // disk state corrupted
    }

    if (disk_state.block_number != expected_parent_block) {
        fclose(f);
        return false;
    }
    if (memcmp(disk_state.state_hash, expected_parent_hash, 32) != 0) {
        fclose(f);
        return false;
    }

    current_block_state.block_number = expected_parent_block + 1;
    for (int i = 0; i < 32; i++) {
        current_block_state.state_hash[i] = disk_state.state_hash[i] ^ (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
    }
    current_block_state.nonce = disk_state.nonce + 1;
    current_block_state.session_key = (disk_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
    current_block_state.gas_allowance = disk_state.gas_allowance;
    current_block_state.is_committed = true;
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));

    rewind(f);
    size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);

    if (written == 1) {
        append_history_record(filepath, &current_block_state);
        char payload[128];
        snprintf(payload, sizeof(payload), "nonce:%u,gas:%u", current_block_state.nonce, current_block_state.gas_allowance);
        TwoThreeNode *tt_node = blue_box_create_leaf(current_block_state.block_number, payload, 0, NULL);
        blue_box_rbt_insert(current_block_state.block_number, tt_node);
        return true;
    }
    return false;
}

void blue_box_crypt_payload(uint8_t *payload, size_t length) {
    if (!payload || length == 0) return;
    uint64_t temp_key = current_block_state.session_key;
    for (size_t i = 0; i < length; i++) {
        if (i % 8 == 0 && i > 0) {
            temp_key = (temp_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
        }
        payload[i] ^= (uint8_t)(temp_key >> ((i % 8) * 8));
    }
}

bool blue_box_decode_access_code(const char *dial_sequence) {
    if (!dial_sequence) return false;
    if (strncmp(dial_sequence, "*99*", 4) == 0) {
        size_t len = strlen(dial_sequence);
        if (len > 5 && dial_sequence[len - 1] == '#') {
            current_block_state.gas_allowance += 250000;
            return true;
        }
    }
    if (strcmp(dial_sequence, "*72") == 0) {
        centrex_avl = avl_insert(centrex_avl, 72, 1);
        current_block_state.active_trunk_mask |= (1U << 31);
        return true;
    }
    if (strcmp(dial_sequence, "*73") == 0) {
        centrex_avl = avl_insert(centrex_avl, 72, 0);
        current_block_state.active_trunk_mask &= ~(1U << 31);
        return true;
    }
    return false;
}

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

uint32_t blue_box_query_blocks(const char *filepath, const char *field, const char *op, uint64_t value, uint32_t *results_out, uint32_t max_results) {
    if (!filepath || !field || !op || !results_out || max_results == 0) return 0;

    // RBT-Optimized Primary Key query lookup
    if (strcmp(field, "block_number") == 0 && strcmp(op, "=") == 0) {
        TwoThreeNode *node = blue_box_rbt_lookup((uint32_t)value);
        if (node) {
            results_out[0] = (uint32_t)value;
            return 1;
        }
        return 0;
    }

    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "rb");
    if (!hf) return 0;

    flock(fileno(hf), LOCK_SH);
    BlueBoxBlockState state;
    uint32_t count = 0;

    while (fread(&state, sizeof(BlueBoxBlockState), 1, hf) == 1 && count < max_results) {
        uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        if (calc != state.checksum || !state.is_committed) continue;

        uint64_t field_val = 0;
        if (strcmp(field, "block_number") == 0) {
            field_val = state.block_number;
        } else if (strcmp(field, "active_trunk_mask") == 0) {
            field_val = state.active_trunk_mask;
        } else if (strcmp(field, "nonce") == 0) {
            field_val = state.nonce;
        } else if (strcmp(field, "gas_allowance") == 0) {
            field_val = state.gas_allowance;
        } else {
            continue; // unsupported column
        }

        bool match = false;
        if (strcmp(op, "=") == 0) {
            match = (field_val == value);
        } else if (strcmp(op, ">") == 0) {
            match = (field_val > value);
        } else if (strcmp(op, "<") == 0) {
            match = (field_val < value);
        } else if (strcmp(op, "&") == 0) {
            match = ((field_val & value) == value);
        }

        if (match) {
            results_out[count++] = state.block_number;
        }
    }

    fclose(hf);
    return count;
}

bool blue_box_update_block_gas(const char *filepath, uint32_t block_number, uint32_t new_gas) {
    if (!filepath) return false;
    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "r+b");
    if (!hf) return false;

    flock(fileno(hf), LOCK_EX);
    BlueBoxBlockState state;
    bool found = false;
    long offset = 0;

    while (fread(&state, sizeof(BlueBoxBlockState), 1, hf) == 1) {
        uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        if (calc == state.checksum && state.block_number == block_number && state.is_committed) {
            state.gas_allowance = new_gas;
            state.checksum = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
            fseek(hf, offset, SEEK_SET);
            fwrite(&state, sizeof(BlueBoxBlockState), 1, hf);
            found = true;
            break;
        }
        offset = ftell(hf);
    }
    fclose(hf);

    if (found) {
        TwoThreeNode *node = blue_box_rbt_lookup(block_number);
        if (node) {
            char payload[128];
            snprintf(payload, sizeof(payload), "nonce:%u,gas:%u", state.nonce, new_gas);
            if (node->is_leaf) {
                if (node->keys[0] == block_number) {
                    strcpy(node->values[0], payload);
                } else if (node->num_keys == 2 && node->keys[1] == block_number) {
                    strcpy(node->values[1], payload);
                }
                blue_box_update_node_hash(node);
            }
        }
    }
    return found;
}

bool blue_box_delete_block(const char *filepath, uint32_t block_number) {
    if (!filepath) return false;
    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "r+b");
    if (!hf) return false;

    flock(fileno(hf), LOCK_EX);
    BlueBoxBlockState state;
    bool found = false;
    long offset = 0;

    while (fread(&state, sizeof(BlueBoxBlockState), 1, hf) == 1) {
        uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        if (calc == state.checksum && state.block_number == block_number && state.is_committed) {
            state.is_committed = false;
            state.checksum = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
            fseek(hf, offset, SEEK_SET);
            fwrite(&state, sizeof(BlueBoxBlockState), 1, hf);
            found = true;
            break;
        }
        offset = ftell(hf);
    }
    fclose(hf);
    return found;
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
        snprintf(payload, sizeof(payload), "nonce:%u,gas:%u", state->nonce, state->gas_allowance);
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

uint64_t blue_box_aggregate_blocks(const char *filepath, const char *field, const char *agg_func) {
    if (!filepath || !field || !agg_func) return 0;

    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "rb");
    if (!hf) return 0;

    flock(fileno(hf), LOCK_SH);
    BlueBoxBlockState state;
    uint64_t sum = 0;
    uint64_t count = 0;
    uint64_t min_val = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t max_val = 0;

    while (fread(&state, sizeof(BlueBoxBlockState), 1, hf) == 1) {
        uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        if (calc != state.checksum || !state.is_committed) continue;

        uint64_t field_val = 0;
        if (strcmp(field, "block_number") == 0) {
            field_val = state.block_number;
        } else if (strcmp(field, "active_trunk_mask") == 0) {
            field_val = state.active_trunk_mask;
        } else if (strcmp(field, "nonce") == 0) {
            field_val = state.nonce;
        } else if (strcmp(field, "gas_allowance") == 0) {
            field_val = state.gas_allowance;
        } else {
            continue;
        }

        sum += field_val;
        count++;
        if (field_val < min_val) min_val = field_val;
        if (field_val > max_val) max_val = field_val;
    }
    fclose(hf);

    if (count == 0) return 0;
    if (strcmp(agg_func, "SUM") == 0) return sum;
    if (strcmp(agg_func, "AVG") == 0) return sum / count;
    if (strcmp(agg_func, "MIN") == 0) return min_val;
    if (strcmp(agg_func, "MAX") == 0) return max_val;
    if (strcmp(agg_func, "COUNT") == 0) return count;
    return 0;
}

// Redundant bottom AVL declarations removed to consolidate at the top.

uint32_t blue_box_query_blocks_sorted(const char *filepath, const char *field, const char *op, uint64_t value, const char *sort_field, uint32_t *results_out, uint32_t max_results) {
    if (!filepath || !field || !op || !results_out || max_results == 0 || !sort_field) return 0;

    char hist_path[512];
    snprintf(hist_path, sizeof(hist_path), "%s.hist", filepath);
    FILE *hf = fopen(hist_path, "rb");
    if (!hf) return 0;

    flock(fileno(hf), LOCK_SH);
    BlueBoxBlockState state;
    uint32_t match_count = 0;
    AvlNode *sort_tree = NULL;

    while (fread(&state, sizeof(BlueBoxBlockState), 1, hf) == 1) {
        uint32_t calc = calculate_crc32((const uint8_t *)&state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        if (calc != state.checksum || !state.is_committed) continue;

        uint64_t field_val = 0;
        if (strcmp(field, "block_number") == 0) {
            field_val = state.block_number;
        } else if (strcmp(field, "active_trunk_mask") == 0) {
            field_val = state.active_trunk_mask;
        } else if (strcmp(field, "nonce") == 0) {
            field_val = state.nonce;
        } else if (strcmp(field, "gas_allowance") == 0) {
            field_val = state.gas_allowance;
        } else {
            continue;
        }

        bool match = false;
        if (strcmp(op, "=") == 0) {
            match = (field_val == value);
        } else if (strcmp(op, ">") == 0) {
            match = (field_val > value);
        } else if (strcmp(op, "<") == 0) {
            match = (field_val < value);
        } else if (strcmp(op, "&") == 0) {
            match = ((field_val & value) == value);
        }

        if (match) {
            uint32_t sort_val = 0;
            if (strcmp(sort_field, "block_number") == 0) {
                sort_val = state.block_number;
            } else if (strcmp(sort_field, "active_trunk_mask") == 0) {
                sort_val = state.active_trunk_mask;
            } else if (strcmp(sort_field, "nonce") == 0) {
                sort_val = state.nonce;
            } else if (strcmp(sort_field, "gas_allowance") == 0) {
                sort_val = state.gas_allowance;
            }
            sort_tree = avl_insert(sort_tree, sort_val, state.block_number);
            match_count++;
        }
    }
    fclose(hf);

    if (match_count == 0) return 0;

    uint32_t idx = 0;
    avl_inorder(sort_tree, results_out, &idx, max_results);
    avl_free(sort_tree);
    return idx;
}

static TsfiZmmVmState blue_box_zmm_state;
static bool blue_box_zmm_initialized = false;

int blue_box_dispatch_zmm_rpc(const char *json_in, char *output_buf, size_t out_max) {
    if (!blue_box_zmm_initialized) {
        tsfi_zmm_vm_init(&blue_box_zmm_state);
        blue_box_zmm_initialized = true;
    }
    return tsfi_zmm_rpc_dispatch(&blue_box_zmm_state, json_in, output_buf, out_max);
}

size_t blue_box_citrix_compress_frame(const uint8_t *fb, size_t size, uint8_t *compressed_out, size_t max_out) {
    if (!fb || size == 0 || !compressed_out || max_out == 0) return 0;
    size_t write_idx = 0;
    size_t read_idx = 0;
    while (read_idx < size && write_idx + 2 < max_out) {
        uint8_t val = fb[read_idx];
        uint8_t run_len = 1;
        while (read_idx + run_len < size && fb[read_idx + run_len] == val && run_len < 255) {
            run_len++;
        }
        compressed_out[write_idx++] = run_len;
        compressed_out[write_idx++] = val;
        read_idx += run_len;
    }
    return write_idx;
}

size_t blue_box_citrix_compress_audio(const float *samples, size_t count, uint8_t *compressed_out, size_t max_out) {
    if (!samples || count == 0 || !compressed_out || max_out == 0) return 0;
    size_t write_bytes = 0;
    float last_val = 0.0f;
    for (size_t i = 0; i < count && write_bytes < max_out; i++) {
        float delta = samples[i] - last_val;
        int8_t step = (int8_t)(delta * 127.0f);
        compressed_out[write_bytes++] = (uint8_t)step;
        last_val = last_val + ((float)step / 127.0f);
    }
    return write_bytes;
}
