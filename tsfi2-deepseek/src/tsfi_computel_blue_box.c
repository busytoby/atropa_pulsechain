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

#include "tsfi_computel_blue_box.h"



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

AvlNode *centrex_avl = NULL;

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

uint32_t calculate_crc32(const uint8_t *data, size_t length) {
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



BlueBoxBlockState current_block_state = {0, {0}, 0, 0, 0, 0, false, "", 0.0f, 0};

void blue_box_init_block(uint32_t block_number, const uint8_t *initial_hash) {
    current_block_state.block_number = block_number;
    current_block_state.active_trunk_mask = 0;
    current_block_state.nonce = 0;
    current_block_state.session_key = 0xDEADC0DE95346795ULL;
    current_block_state.gas_allowance = 500000;
    current_block_state.is_committed = false;
    current_block_state.unicode_desc[0] = '\0';
    current_block_state.synth_frequency = 0.0f;
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

void blue_box_set_block_unicode_synth(const char *desc, float freq) {
    if (desc) {
        strncpy(current_block_state.unicode_desc, desc, sizeof(current_block_state.unicode_desc) - 1);
        current_block_state.unicode_desc[sizeof(current_block_state.unicode_desc) - 1] = '\0';
    } else {
        current_block_state.unicode_desc[0] = '\0';
    }
    current_block_state.synth_frequency = freq;
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

static double sf_phase = 0.0;
static double mf_phase_f1 = 0.0;
static double mf_phase_f2 = 0.0;
static double red_box_phase_f1 = 0.0;
static double red_box_phase_f2 = 0.0;

/* Generates 2600 Hz SF tone to seize simulated trunk line - phase continuous */
void generate_sf_seizure(float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) {
        buffer[i] = (float)sin(sf_phase);
        sf_phase += 2.0 * M_PI * 2600.0 / SAMPLE_RATE;
        if (sf_phase > 2.0 * M_PI) sf_phase -= 2.0 * M_PI;
    }
}

/* Generates MF tones to route calls inside tandem trunk switch matrices - phase continuous */
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
        buffer[i] = (float)((sin(mf_phase_f1) + sin(mf_phase_f2)) * 0.5);
        mf_phase_f1 += 2.0 * M_PI * f1 / SAMPLE_RATE;
        mf_phase_f2 += 2.0 * M_PI * f2 / SAMPLE_RATE;
        if (mf_phase_f1 > 2.0 * M_PI) mf_phase_f1 -= 2.0 * M_PI;
        if (mf_phase_f2 > 2.0 * M_PI) mf_phase_f2 -= 2.0 * M_PI;
    }

    return true;
}

/* Generates Red Box payphone coin tones (1700 Hz + 2200 Hz beeps) - phase continuous */
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
            float decay = (float)exp(-3.0 * (double)i / beep_samples);
            buffer[offset + i] = (float)((sin(red_box_phase_f1) + sin(red_box_phase_f2)) * 0.5 * decay);
            
            // Emulate transient gong chirp in first 40 samples
            if (i < 40) {
                buffer[offset + i] += (float)(((double)rand() / RAND_MAX) * 0.15);
            }

            red_box_phase_f1 += 2.0 * M_PI * 1700.0 / SAMPLE_RATE;
            red_box_phase_f2 += 2.0 * M_PI * 2200.0 / SAMPLE_RATE;
            if (red_box_phase_f1 > 2.0 * M_PI) red_box_phase_f1 -= 2.0 * M_PI;
            if (red_box_phase_f2 > 2.0 * M_PI) red_box_phase_f2 -= 2.0 * M_PI;
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

static bool goertzel_detect(const float *samples, int num_samples, float target_freq) {
    float s_prev = 0.0f;
    float s_prev2 = 0.0f;
    float normalized_freq = target_freq / SAMPLE_RATE;
    float coeff = 2.0f * (float)cos(2.0 * M_PI * normalized_freq);
    for (int i = 0; i < num_samples; i++) {
        float s = samples[i] + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }
    float power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
    return (power > (float)num_samples * 0.1f);
}

bool blue_box_validate_slug(const float *samples, int num_samples) {
    if (!samples || num_samples < 200) return false;
    
    // 1. Transient check (Variance in the first 40 samples to detect impact chirp)
    float sum = 0.0f;
    for (int i = 0; i < 40; i++) sum += samples[i];
    float mean = sum / 40.0f;
    float variance = 0.0f;
    for (int i = 0; i < 40; i++) {
        float diff = samples[i] - mean;
        variance += diff * diff;
    }
    variance /= 40.0f;
    if (variance < 0.001f) return false; // Fail if flat/electronic sine wave

    // 2. Exponential Decay check (amplitude comparison)
    float amp_start = 0.0f;
    float amp_end = 0.0f;
    int half = num_samples / 2;
    for (int i = 0; i < half; i++) {
        amp_start += (float)fabs(samples[i]);
        amp_end += (float)fabs(samples[i + half]);
    }
    if (amp_start == 0.0f) return false;
    float ratio = amp_end / amp_start;
    
    // Metallic damping ratio must be between 0.1 and 0.85
    if (ratio < 0.1f || ratio > 0.85f) {
        return false;
    }
    return true;
}

bool blue_box_detect_and_accumulate(const float *samples, int num_samples) {
    if (!samples || num_samples <= 0) return false;
    
    // Skip SF 2600Hz lines from slug validation; only validate coin tones
    if (goertzel_detect(samples, num_samples, 2600.0f)) {
        blue_box_accumulate_state(2600);
        return true;
    }
    
    if (num_samples >= 200) {
        if (!blue_box_validate_slug(samples, num_samples)) {
            // Flag fraud by zeroing active trunk mask
            current_block_state.active_trunk_mask = 0;
            return false;
        }
    }

    float targets[6] = {700.0f, 900.0f, 1100.0f, 1300.0f, 1500.0f, 1700.0f};
    uint64_t found_mask = 0;
    for (int i = 0; i < 6; i++) {
        if (goertzel_detect(samples, num_samples, targets[i])) {
            found_mask |= (1ULL << i);
            blue_box_accumulate_state((uint64_t)targets[i]);
        }
    }
    return found_mask != 0;
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

void blue_box_recover_wal(const char *filepath) {
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
                snprintf(payload, sizeof(payload), "nonce:%u,gas:%u,unicode:%s,synth:%.2f", hist_state.nonce, hist_state.gas_allowance, hist_state.unicode_desc, hist_state.synth_frequency);
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
            snprintf(payload, sizeof(payload), "nonce:%u,gas:%u,unicode:%s,synth:%.2f", current_block_state.nonce, current_block_state.gas_allowance, current_block_state.unicode_desc, current_block_state.synth_frequency);
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
        snprintf(payload, sizeof(payload), "nonce:%u,gas:%u,unicode:%s,synth:%.2f", current_block_state.nonce, current_block_state.gas_allowance, current_block_state.unicode_desc, current_block_state.synth_frequency);
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
        } else if (strcmp(field, "synth_frequency") == 0) {
            field_val = (uint64_t)state.synth_frequency;
        } else if (strcmp(field, "unicode_desc") == 0) {
            if (strcmp(op, "=") == 0) {
                if (calculate_crc32((const uint8_t *)state.unicode_desc, strlen(state.unicode_desc)) == value) {
                    results_out[count++] = state.block_number;
                }
                continue;
            }
            continue;
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
            snprintf(payload, sizeof(payload), "nonce:%u,gas:%u,unicode:%s,synth:%.2f", state.nonce, new_gas, state.unicode_desc, state.synth_frequency);
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

// 1. Dynamic Rate Matrix
static uint32_t trunk_rates[32] = {0}; // Maps trunk 800-831 to rates (0-31 offset)

void blue_box_centrex_set_trunk_rate(uint32_t trunk_id, uint32_t rate_per_min) {
    if (trunk_id >= 800 && trunk_id <= 831) {
        trunk_rates[trunk_id - 800] = rate_per_min;
    }
}

uint32_t blue_box_centrex_get_trunk_rate(uint32_t trunk_id) {
    if (trunk_id >= 800 && trunk_id <= 831) {
        return trunk_rates[trunk_id - 800];
    }
    return 0;
}

// 2. Multi-Coin Accumulation
void blue_box_rotate_key_on_coin(int denomination) {
    uint64_t multiplier = 1103515245ULL;
    uint64_t increment = 12345ULL;
    current_block_state.session_key = ((current_block_state.session_key ^ (uint64_t)denomination) * multiplier + increment) & 0xFFFFFFFFFFFFFFFFULL;
}

void blue_box_accumulate_coin(int denomination) {
    uint64_t coin_val = (uint64_t)denomination;
    accumulator_register = (accumulator_register * 33 + coin_val) % MotzkinPrime;
    for (int i = 0; i < 8; i++) {
        current_block_state.state_hash[i] ^= (uint8_t)(accumulator_register >> (i * 8));
    }
    blue_box_rotate_key_on_coin(denomination);
}

// 3. Signed ERC20 Transaction Bridge
bool blue_box_generate_erc20_tx(char *tx_buf, size_t max_len) {
    if (!tx_buf || max_len < 256) return false;
    snprintf(tx_buf, max_len, 
             "{\"jsonrpc\":\"2.0\",\"method\":\"erc20_transfer\",\"params\":{\"amount\":%u,\"sig_hash\":\"%02x%02x%02x%02x\"},\"id\":1}",
             current_block_state.gas_allowance, 
             current_block_state.state_hash[0], current_block_state.state_hash[1],
             current_block_state.state_hash[2], current_block_state.state_hash[3]);
    return true;
}

// 4. Real-time Balance Depletion
bool blue_box_deplete_session_gas(uint32_t trunk_id, uint32_t active_seconds) {
    uint32_t rate = blue_box_centrex_get_trunk_rate(trunk_id);
    uint32_t cost = (rate * active_seconds) / 60;
    
    // Scale cost based on live PLL hardware phase lock deviation (0xF125)
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    uint64_t actual_pll_deviation = lau_yul_thunk_sload(0xF125);
    int64_t s_dev = (int64_t)actual_pll_deviation;
    uint64_t pll_abs = s_dev >= 0 ? (uint64_t)s_dev : (uint64_t)(-s_dev);
    
    if (pll_abs > 0) {
        uint64_t multiplier = 1 + (pll_abs / 500000);
        if (multiplier > 10) multiplier = 10;
        cost *= multiplier;
    }
    
    if (current_block_state.gas_allowance >= cost) {
        current_block_state.gas_allowance -= cost;
        return true;
    } else {
        current_block_state.gas_allowance = 0;
        current_block_state.active_trunk_mask &= ~(1U << (trunk_id - 800)); // Disconnect
        return false;
    }
}

// 5. On-Chain Tariff Negotiation
bool blue_box_negotiate_tariff(uint32_t trunk_id, uint32_t *rate_out) {
    if (!rate_out) return false;
    char json_in[256];
    snprintf(json_in, sizeof(json_in), "{\"jsonrpc\":\"2.0\",\"method\":\"tariffs_query\",\"params\":{\"trunk_id\":%u},\"id\":1}", trunk_id);
    char rpc_out[1024];
    int ok = blue_box_dispatch_zmm_rpc(json_in, rpc_out, sizeof(rpc_out));
    if (ok == 1) {
        uint32_t rate = 0;
        char *result_ptr = strstr(rpc_out, "\"rate\"");
        if (result_ptr) {
            result_ptr = strchr(result_ptr, ':');
            if (result_ptr) {
                rate = (uint32_t)atoi(result_ptr + 1);
                *rate_out = rate;
                blue_box_centrex_set_trunk_rate(trunk_id, rate);
                return true;
            }
        }
    }
    // Fallback
    *rate_out = blue_box_centrex_get_trunk_rate(trunk_id);
    return true;
}

// 6. UDP Tone Streaming
bool blue_box_send_udp_tone(uint32_t port, const float *samples, size_t count) {
    if (!samples || count == 0 || port == 0) return false;
    uint8_t comp_buf[4096];
    size_t comp_size = blue_box_citrix_compress_audio(samples, count, comp_buf, sizeof(comp_buf));
    if (comp_size == 0) return false;
    
    // Simulate raw socket transmit loops
    printf("[UDP STREAM] Port %u: Transmitted %lu compressed audio bytes.\n", port, comp_size);
    return true;
}

// 7. Kermit-over-UDP Packetization
bool blue_box_kermit_send_udp(uint32_t port, const uint8_t *packet, size_t len) {
    if (!packet || len == 0 || port == 0) return false;
    // Prepend Kermit over UDP magic word (0x4B55 = "KU")
    uint8_t udp_payload[2048];
    if (len + 2 > sizeof(udp_payload)) return false;
    udp_payload[0] = 0x4B; // 'K'
    udp_payload[1] = 0x55; // 'U'
    memcpy(udp_payload + 2, packet, len);
    printf("[UDP KERMIT] Port %u: Transmitted %lu bytes payload.\n", port, len + 2);
    return true;
}

// 8. Real-time UDP Billing Alert
bool blue_box_send_udp_billing_alert(uint32_t port) {
    if (port == 0) return false;
    char alert_buf[256];
    if (!blue_box_generate_erc20_tx(alert_buf, sizeof(alert_buf))) return false;
    printf("[UDP BILLING] Port %u: Dispatched alert: %s\n", port, alert_buf);
    return true;
}

// 9. Closed-Loop Ouroboros Feedback
void blue_box_ouroboros_tick(void) {
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    uint64_t pos = lau_yul_thunk_sload(0xF100);
    uint64_t neg = lau_yul_thunk_sload(0xF101);
    
    int64_t feedback = (int64_t)pos - (int64_t)neg;
    uint64_t base = current_block_state.nonce ? current_block_state.nonce : 3;
    uint64_t signal = feedback > 0 ? (uint64_t)feedback : (uint64_t)(-feedback);
    if (signal == 0) signal = 1;
    
    uint64_t next_signal = (signal * base) % 953467954114363ULL; // MotzkinPrime
    
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    uint8_t yul_cd[36] = {0xe3, 0x99, 0xf0, 0xe0};
    yul_cd[35] = (uint8_t)(next_signal & 0xFF);
    yul_cd[34] = (uint8_t)((next_signal >> 8) & 0xFF);
    yul_cd[33] = (uint8_t)((next_signal >> 16) & 0xFF);
    yul_cd[32] = (uint8_t)((next_signal >> 24) & 0xFF);
    
    uint8_t yul_ret[32];
    size_t yul_ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", yul_cd, 36, yul_ret, &yul_ret_len);
    
    blue_box_accumulate_state(next_signal);
}

// 10. BTC Script Dual Stack verification (A side standard crypto, B side PLL hardware telemetry)
bool blue_box_verify_dual_stack(const uint8_t *sig, size_t sig_len, const uint8_t *pubkey, size_t pubkey_len, uint64_t max_pll_deviation) {
    if (!sig || sig_len == 0 || !pubkey || pubkey_len == 0) return false;
    
    // Simulating A-side standard crypto validation (OP_CHECKSIG)
    bool a_side_ok = (sig_len >= 4 && pubkey_len >= 4);
    
    // B-side low-level PLL hardware check from WinchesterMQ register 0xF125
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    uint64_t actual_pll_deviation = lau_yul_thunk_sload(0xF125);
    
    // Check if dynamic phase deviation is within safe limits (absolute value check)
    int64_t s_dev = (int64_t)actual_pll_deviation;
    uint64_t pll_abs = s_dev >= 0 ? (uint64_t)s_dev : (uint64_t)(-s_dev);
    bool b_side_ok = (pll_abs <= max_pll_deviation);
    
    printf("[DUAL STACK] A-side (Crypto): %s | B-side (PLL Dev: %lu, Limit: %lu): %s\n",
           a_side_ok ? "PASS" : "FAIL", pll_abs, max_pll_deviation, b_side_ok ? "PASS" : "FAIL");
           
    return (a_side_ok && b_side_ok);
}

// 11. MF/FSK Tone Generator & Dynamic Vocable Synthesis
bool blue_box_generate_tone(uint32_t freq1, uint32_t freq2, float *samples_out, size_t count) {
    if (!samples_out || count == 0) return false;
    
    // Write frequencies to WinchesterMQ VM registers via generateTone thunk selector
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    uint8_t tone_cd[68] = {0x0f, 0xf1, 0x10, 0x00};
    // Pack freq1 and freq2 as big-endian 32-byte arguments
    tone_cd[35] = (uint8_t)(freq1 & 0xFF);
    tone_cd[34] = (uint8_t)((freq1 >> 8) & 0xFF);
    tone_cd[33] = (uint8_t)((freq1 >> 16) & 0xFF);
    tone_cd[32] = (uint8_t)((freq1 >> 24) & 0xFF);
    tone_cd[67] = (uint8_t)(freq2 & 0xFF);
    tone_cd[66] = (uint8_t)((freq2 >> 8) & 0xFF);
    tone_cd[65] = (uint8_t)((freq2 >> 16) & 0xFF);
    tone_cd[64] = (uint8_t)((freq2 >> 24) & 0xFF);
    
    uint8_t tone_ret[32];
    size_t tone_ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", tone_cd, 68, tone_ret, &tone_ret_len);
    
    // Synthesize dual sine wave audio samples
    double fs = 8000.0; // Standard telephony sampling rate (8 kHz)
    for (size_t n = 0; n < count; n++) {
        double t = (double)n / fs;
        samples_out[n] = 0.5 * (sin(2.0 * M_PI * freq1 * t) + sin(2.0 * M_PI * freq2 * t));
    }
    return true;
}

// 12. Visual Coverage & Symmetry Telemetry Classifier
bool blue_box_evaluate_visual_coverage(const float *x_coords, const float *y_coords, size_t count, float *coverage_out, float *symmetry_out) {
    if (!x_coords || !y_coords || count == 0 || !coverage_out || !symmetry_out) return false;
    
    #define GRID_SIZE 80
    uint8_t grid[GRID_SIZE][GRID_SIZE];
    memset(grid, 0, sizeof(grid));
    
    size_t active_pixels = 0;
    for (size_t i = 0; i < count; i++) {
        // Normalize coordinates from [-1.0, 1.0] to [0, GRID_SIZE-1]
        int px = (int)((x_coords[i] + 1.0f) * 0.5f * (GRID_SIZE - 1));
        int py = (int)((y_coords[i] + 1.0f) * 0.5f * (GRID_SIZE - 1));
        if (px >= 0 && px < GRID_SIZE && py >= 0 && py < GRID_SIZE) {
            if (grid[py][px] == 0) {
                grid[py][px] = 1;
                active_pixels++;
            }
        }
    }
    
    *coverage_out = (float)active_pixels / (GRID_SIZE * GRID_SIZE);
    
    // Evaluate symmetry by reflecting along the vertical Y-axis reflection plane
    size_t sym_hits = 0;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE / 2; x++) {
            if (grid[y][x] == grid[y][GRID_SIZE - 1 - x]) {
                sym_hits++;
            }
        }
    }
    *symmetry_out = (float)sym_hits / (GRID_SIZE * (GRID_SIZE / 2));
    return true;
}

// 13. MF Dialing Sequence State Machine & Router
static uint32_t g_mf_state = 0; // 0 = idle, 1 = dialing, 2 = routed
static char g_mf_buffer[32] = {0};

bool blue_box_dial_mf_digit(char digit) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    if (digit == 'K') { // Keypulse (KP) initiates dialing
        g_mf_state = 1;
        memset(g_mf_buffer, 0, sizeof(g_mf_buffer));
        lau_yul_thunk_sstore(0xF130, 1);
        lau_yul_thunk_sstore(0xF131, 0);
        printf("[MF STATE] KP tone received. Resetting digit dialing buffer.\n");
        return true;
    }
    if (g_mf_state == 1) {
        if (digit >= '0' && digit <= '9') {
            size_t len = strlen(g_mf_buffer);
            if (len < sizeof(g_mf_buffer) - 1) {
                g_mf_buffer[len] = digit;
                // Accumulate hash in VM storage
                uint64_t hash = 0;
                for (size_t i = 0; g_mf_buffer[i]; i++) {
                    hash = hash * 10 + (g_mf_buffer[i] - '0');
                }
                lau_yul_thunk_sstore(0xF131, hash);
                printf("[MF STATE] Digit '%c' appended. Buffer: %s (VM Hash: %lu)\n", digit, g_mf_buffer, hash);
            }
            return true;
        }
        if (digit == 'S') { // Start (ST) terminates and routes
            g_mf_state = 2;
            lau_yul_thunk_sstore(0xF130, 2);
            printf("[MF STATE] ST tone received. Dialing complete. Routing trunk call to: %s\n", g_mf_buffer);
            return true;
        }
    }
    return false;
}

// 14. Formant Vowel Vocable Synthesizer
bool blue_box_synthesize_vowel(char vowel, float *samples_out, size_t count) {
    if (!samples_out || count == 0) return false;
    
    // Formant definitions: F1, F2, F3 frequencies per vowel vocable
    uint32_t f1 = 0, f2 = 0, f3 = 0;
    switch (vowel) {
        case 'A': f1 = 730;  f2 = 1090; f3 = 2440; break;
        case 'E': f1 = 530;  f2 = 1840; f3 = 2480; break;
        case 'I': f1 = 270;  f2 = 2290; f3 = 3010; break;
        case 'O': f1 = 570;  f2 = 840;  f3 = 2410; break;
        case 'U': f1 = 300;  f2 = 870;  f3 = 2240; break;
        default: return false;
    }
    
    double fs = 8000.0;
    for (size_t n = 0; n < count; n++) {
        double t = (double)n / fs;
        // Superpose the three formants to synthesize human vocable
        samples_out[n] = 0.33f * (sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t) + sin(2.0 * M_PI * f3 * t));
    }
    return true;
}

// 15. Wink-Start Handshaking State Machine
bool blue_box_trigger_wink(uint32_t duration_ms) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    bool valid = (duration_ms >= 150 && duration_ms <= 300);
    lau_yul_thunk_sstore(0xF135, valid ? 1 : 0);
    printf("[WINK] Wink pulse processed (duration: %u ms). Valid: %s\n", duration_ms, valid ? "YES" : "NO");
    return valid;
}

// 16. Trunk Line Splitting & 2600 Hz Notch Filter
bool blue_box_apply_notch_filter(const float *samples_in, float *samples_out, size_t count, bool split_active) {
    if (!samples_in || !samples_out || count == 0) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF136, split_active ? 1 : 0);
    
    if (!split_active) {
        memcpy(samples_out, samples_in, count * sizeof(float));
        return true;
    }
    
    // 2600 Hz notch filter parameters at 8000 Hz sample rate
    double w0 = 2.0 * M_PI * 2600.0 / 8000.0;
    double cos_w0 = cos(w0);
    double r = 0.85;
    
    // Filter memory state (2 delays)
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
    
    for (size_t n = 0; n < count; n++) {
        float x = samples_in[n];
        float y = (float)(x - 2.0 * cos_w0 * x1 + x2 + 2.0 * r * cos_w0 * y1 - r * r * y2);
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        samples_out[n] = y;
    }
    
    return true;
}

// 17. BGP Peer Table Management & Precedence Routing
#define MAX_BGP_PEERS 32

typedef struct {
    uint32_t peer_ip;
    uint16_t peer_as;
    uint32_t precedence;
    uint32_t latency_ms;
} BGPPeer;

static BGPPeer g_bgp_peers[MAX_BGP_PEERS];
static size_t g_bgp_peer_count = 0;

bool blue_box_add_bgp_peer(uint32_t peer_ip, uint16_t peer_as, uint32_t precedence, uint32_t latency_ms) {
    // Check if peer already exists, update if so
    for (size_t i = 0; i < g_bgp_peer_count; i++) {
        if (g_bgp_peers[i].peer_ip == peer_ip) {
            g_bgp_peers[i].peer_as = peer_as;
            g_bgp_peers[i].precedence = precedence;
            g_bgp_peers[i].latency_ms = latency_ms;
            printf("[BGP PEER] Updated peer IP: 0x%X (AS: %u, Precedence: %u, Latency: %u ms)\n",
                   peer_ip, peer_as, precedence, latency_ms);
            return true;
        }
    }
    if (g_bgp_peer_count >= MAX_BGP_PEERS) return false;
    g_bgp_peers[g_bgp_peer_count].peer_ip = peer_ip;
    g_bgp_peers[g_bgp_peer_count].peer_as = peer_as;
    g_bgp_peers[g_bgp_peer_count].precedence = precedence;
    g_bgp_peers[g_bgp_peer_count].latency_ms = latency_ms;
    g_bgp_peer_count++;
    printf("[BGP PEER] Added new peer IP: 0x%X (AS: %u, Precedence: %u, Latency: %u ms)\n",
           peer_ip, peer_as, precedence, latency_ms);
    return true;
}

bool blue_box_get_bgp_peer(uint32_t peer_ip, uint32_t *precedence_out, uint32_t *latency_out) {
    if (!precedence_out || !latency_out) return false;
    for (size_t i = 0; i < g_bgp_peer_count; i++) {
        if (g_bgp_peers[i].peer_ip == peer_ip) {
            *precedence_out = g_bgp_peers[i].precedence;
            *latency_out = g_bgp_peers[i].latency_ms;
            return true;
        }
    }
    return false;
}

uint32_t blue_box_query_bgp_peers_by_precedence(uint32_t precedence, uint32_t *ips_out, uint32_t max_results) {
    if (!ips_out || max_results == 0) return 0;
    uint32_t count = 0;
    for (size_t i = 0; i < g_bgp_peer_count && count < max_results; i++) {
        if (g_bgp_peers[i].precedence == precedence) {
            ips_out[count++] = g_bgp_peers[i].peer_ip;
        }
    }
    return count;
}

// 18. GGUF BGP Layer Routing
typedef struct {
    char layer_name[64];
    uint32_t peer_ip;
    uint32_t size_bytes;
} GGUFLayerRoute;

#define MAX_GGUF_ROUTES 64
static GGUFLayerRoute g_gguf_routes[MAX_GGUF_ROUTES];
static size_t g_gguf_route_count = 0;

bool blue_box_add_gguf_layer_route(const char *layer_name, uint32_t peer_ip, uint32_t size_bytes) {
    if (!layer_name || g_gguf_route_count >= MAX_GGUF_ROUTES) return false;
    for (size_t i = 0; i < g_gguf_route_count; i++) {
        if (strcmp(g_gguf_routes[i].layer_name, layer_name) == 0 && g_gguf_routes[i].peer_ip == peer_ip) {
            g_gguf_routes[i].size_bytes = size_bytes;
            return true;
        }
    }
    strncpy(g_gguf_routes[g_gguf_route_count].layer_name, layer_name, 63);
    g_gguf_routes[g_gguf_route_count].layer_name[63] = '\0';
    g_gguf_routes[g_gguf_route_count].peer_ip = peer_ip;
    g_gguf_routes[g_gguf_route_count].size_bytes = size_bytes;
    g_gguf_route_count++;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF160, g_gguf_route_count);
    return true;
}

uint32_t blue_box_select_gguf_layer_peer(const char *layer_name) {
    if (!layer_name) return 0;
    uint32_t best_peer_ip = 0;
    double best_cost = 1e18;
    
    for (size_t i = 0; i < g_gguf_route_count; i++) {
        if (strcmp(g_gguf_routes[i].layer_name, layer_name) == 0) {
            uint32_t prec = 0, lat = 0;
            if (!blue_box_get_bgp_peer(g_gguf_routes[i].peer_ip, &prec, &lat)) {
                lat = 100;
            }
            double cost = (double)lat + ((double)g_gguf_routes[i].size_bytes / 100000.0);
            if (cost < best_cost) {
                best_cost = cost;
                best_peer_ip = g_gguf_routes[i].peer_ip;
            }
        }
    }
    return best_peer_ip;
}

// 19. Black Box Line Voltage & Billing Status Simulation
bool blue_box_simulate_black_box(float resistance_ohms, uint32_t *voltage_out, bool *billing_active_out) {
    if (!voltage_out || !billing_active_out) return false;
    
    // V_loop = 48V * R / (R + 1000)
    float v_loop = 48.0f * resistance_ohms / (resistance_ohms + 1000.0f);
    uint32_t v_rounded = (uint32_t)(v_loop + 0.5f);
    *voltage_out = v_rounded;
    
    // Black Box threshold window: 10V to 12V suppresses answer-supervision (no billing)
    bool billing_active = true;
    if (v_loop >= 10.0f && v_loop <= 12.0f) {
        billing_active = false;
    } else if (v_loop < 10.0f) {
        // Under 10V is considered on-hook / line-dropped
        billing_active = false;
    }
    
    *billing_active_out = billing_active;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF150, v_rounded);
    lau_yul_thunk_sstore(0xF151, billing_active ? 1 : 0);
    
    printf("[BLACK BOX] Clamped resistance: %.1f ohms. Voltage: %u V. Billing active: %s\n",
           resistance_ohms, v_rounded, billing_active ? "YES" : "NO");
    return true;
}

// 20. Hook Flash Signaling detection and flash counting
static uint32_t g_hook_flash_count = 0;

bool blue_box_trigger_hook_flash(uint32_t duration_ms, bool *flash_detected_out, uint32_t *flash_count_out) {
    if (!flash_detected_out || !flash_count_out) return false;
    
    bool detected = (duration_ms >= 500 && duration_ms <= 1000);
    if (detected) {
        g_hook_flash_count++;
    } else if (duration_ms > 1000) {
        g_hook_flash_count = 0; // complete disconnect
    }
    
    *flash_detected_out = detected;
    *flash_count_out = g_hook_flash_count;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    lau_yul_thunk_sstore(0xF152, detected ? 1 : 0);
    lau_yul_thunk_sstore(0xF153, g_hook_flash_count);
    
    printf("[HOOK FLASH] Pulse processed (duration: %u ms). Detected: %s. Total Flashes: %u\n",
           duration_ms, detected ? "YES" : "NO", g_hook_flash_count);
    return true;
}

// 21. AI Driver Telemetry Monitor and Automated Control Loop
bool blue_box_run_ai_driver(bool enable_ai, uint32_t *command_out) {
    if (!command_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    lau_yul_thunk_sstore(0xF155, enable_ai ? 1 : 0);
    
    if (!enable_ai) {
        *command_out = 0;
        lau_yul_thunk_sstore(0xF156, 0);
        return true;
    }
    
    // Telemetry read
    uint64_t reg_bill = lau_yul_thunk_sload(0xF151);
    uint64_t reg_flash_cnt = lau_yul_thunk_sload(0xF153);
    
    uint32_t cmd = 0;
    if (reg_bill == 1) {
        // Countermeasure: suppress billing by clamping loop resistance
        cmd = 1;
        uint32_t volt = 0;
        bool bill_act = true;
        blue_box_simulate_black_box(300.0f, &volt, &bill_act);
    } else if (reg_bill == 0 && reg_flash_cnt == 0) {
        // Suppressed, but route flash required to verify routing
        cmd = 2;
        bool flash_det = false;
        uint32_t flash_cnt = 0;
        blue_box_trigger_hook_flash(700, &flash_det, &flash_cnt);
    } else {
        cmd = 0; // Idle / system stable
    }
    
    *command_out = cmd;
    lau_yul_thunk_sstore(0xF156, cmd);
    printf("[AI DRIVER] Active: YES. Decision: Command %u executed.\n", cmd);
    return true;
}

// 22. AI Speech Sequencer
bool blue_box_run_ai_speech_sequencer(uint32_t state, char *vowel_sequence_out, size_t max_len) {
    if (!vowel_sequence_out || max_len < 4) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    
    lau_yul_thunk_sstore(0xF170, state);
    
    const char *seq = "";
    switch (state) {
        case 1: seq = "AE"; break; // Greeting
        case 2: seq = "OI"; break; // Alert
        case 3: seq = "AO"; break; // Confirmation
        default: seq = ""; break;
    }
    
    strncpy(vowel_sequence_out, seq, max_len - 1);
    vowel_sequence_out[max_len - 1] = '\0';
    
    // Store the first vowel code in VM register 0xF171
    lau_yul_thunk_sstore(0xF171, vowel_sequence_out[0] ? (uint64_t)vowel_sequence_out[0] : 0);
    
    printf("[AI SPEECH] State: %u. Sequence: \"%s\". Primary Vowel Token: '%c' (VM: %lu)\n",
           state, vowel_sequence_out, vowel_sequence_out[0] ? vowel_sequence_out[0] : ' ',
           vowel_sequence_out[0] ? (uint64_t)vowel_sequence_out[0] : 0);
    return true;
}

// 23. Unified Singular Telecom Dispatch Tick
bool blue_box_unified_tick(uint32_t delta_time_ms) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    // 1. Update System Tick Count
    uint64_t ticks = lau_yul_thunk_sload(0xF180);
    ticks += delta_time_ms;
    lau_yul_thunk_sstore(0xF180, ticks);
    
    // 2. Execute WinchesterMQ Ouroboros Feedback loop
    blue_box_ouroboros_tick();
    
    // 3. Deplete session gas if billing is active
    uint64_t billing_active = lau_yul_thunk_sload(0xF151);
    if (billing_active == 1) {
        uint32_t active_seconds = (delta_time_ms + 999) / 1000;
        blue_box_deplete_session_gas(808, active_seconds);
    }
    
    // 4. Run AI Agent automated control decisions if enabled
    uint64_t ai_enabled = lau_yul_thunk_sload(0xF155);
    if (ai_enabled == 1) {
        uint32_t dummy_cmd = 0;
        blue_box_run_ai_driver(true, &dummy_cmd);
    }
    
    // 5. Track RDBMS commit rate and increment transaction counters
    uint64_t tx_count = lau_yul_thunk_sload(0xF145);
    if (billing_active == 1) {
        tx_count++; // Simulates ongoing database operations
        lau_yul_thunk_sstore(0xF145, tx_count);
    }
    
    printf("[UNIFIED TICK] Delta: %u ms. Total Ticks: %lu. Billing: %s. AI Active: %s\n",
           delta_time_ms, ticks, billing_active ? "YES" : "NO", ai_enabled ? "YES" : "NO");
    
    return true;
}

// 24. Green Box Coin Disposition Control Actions
bool blue_box_trigger_green_box(uint32_t frequency, uint32_t duration_ms, uint32_t *action_out) {
    if (!action_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t blue_box_get_accumulator(void);
    
    uint32_t action = 0;
    bool valid = (duration_ms >= 400 && duration_ms <= 600);
    
    if (valid) {
        if (frequency == 1700) {
            action = 1; // Collect Coin
            // Vault the coin: reset active accumulator to 0
            extern void blue_box_deplete_session_gas_override(void);
            lau_yul_thunk_sstore(0xF120, 0); // resets threat
        } else if (frequency == 2200) {
            action = 2; // Return Coin
            // Return to customer
        }
    }
    
    *action_out = action;
    lau_yul_thunk_sstore(0xF185, action);
    printf("[GREEN BOX] Tone processed: %u Hz (duration: %u ms). Action: %u\n",
           frequency, duration_ms, action);
    return true;
}

// 25. Green Box ERC20 compatible transaction thunk bridge
bool blue_box_green_box_to_erc20(uint32_t action, char *payload_out, size_t max_len) {
    if (!payload_out || max_len < 256) return false;
    
    const char *method = "";
    if (action == 1) {
        method = "erc20_collect";
    } else if (action == 2) {
        method = "erc20_refund";
    } else {
        return false;
    }
    
    snprintf(payload_out, max_len,
             "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":{\"amount\":%u,\"sig_hash\":\"%02x%02x%02x%02x\"},\"id\":1}",
             method,
             current_block_state.gas_allowance,
             current_block_state.state_hash[4], current_block_state.state_hash[5],
             current_block_state.state_hash[6], current_block_state.state_hash[7]);
             
    return true;
}

// 26. Green Box Coin Disposition with Diyat fee calculations
bool blue_box_trigger_green_box_diyat(uint32_t frequency, uint32_t duration_ms, uint32_t *action_out) {
    if (!action_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint32_t action = 0;
    bool valid = (duration_ms >= 400 && duration_ms <= 600);
    
    if (valid) {
        if (frequency == 1700) {
            action = 1; // Collect
            uint64_t session_gas = current_block_state.gas_allowance;
            uint64_t vault = lau_yul_thunk_sload(0xF186);
            lau_yul_thunk_sstore(0xF186, vault + session_gas);
            current_block_state.gas_allowance = 0;
            lau_yul_thunk_sstore(0xF199, 0);
        } else if (frequency == 2200) {
            action = 2; // Return
            uint64_t session_gas = current_block_state.gas_allowance;
            uint64_t rate = lau_yul_thunk_sload(0xF196);
            if (rate == 0) rate = 5; // Default 5%
            
            uint64_t fee = (session_gas * rate) / 100;
            uint64_t refund = session_gas - fee;
            
            uint64_t fee_pool = lau_yul_thunk_sload(0xF195);
            uint64_t refund_pool = lau_yul_thunk_sload(0xF187);
            
            lau_yul_thunk_sstore(0xF195, fee_pool + fee);
            lau_yul_thunk_sstore(0xF187, refund_pool + refund);
            
            uint64_t total_gas = lau_yul_thunk_sload(0xF199);
            lau_yul_thunk_sstore(0xF199, total_gas + refund);
            current_block_state.gas_allowance = total_gas + refund;
        }
    }
    
    *action_out = action;
    lau_yul_thunk_sstore(0xF185, action);
    printf("[GREEN BOX DIYAT] Tone: %u Hz. Action: %u. Fee Pool: %lu. Allowance: %u\n",
           frequency, action, lau_yul_thunk_sload(0xF195), current_block_state.gas_allowance);
    return true;
}
