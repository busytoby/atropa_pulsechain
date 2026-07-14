#ifndef TSFI_COMPUTEL_BLUE_BOX_H
#define TSFI_COMPUTEL_BLUE_BOX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TWO_THREE_HASH_SIZE 32

typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[TWO_THREE_HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

typedef enum { RBT_RED, RBT_BLACK } RbtColor;

#define RBT_MAX_NODES 256

typedef struct RbtNode {
    uint32_t block_number;
    TwoThreeNode *two_three_node;
    RbtColor color;
    struct RbtNode *left;
    struct RbtNode *right;
    struct RbtNode *parent;
} RbtNode;

typedef struct AvlNode {
    uint32_t val;
    uint32_t block_number;
    int height;
    struct AvlNode *left;
    struct AvlNode *right;
} AvlNode;

extern RbtNode *rbt_root;
extern uint32_t rbt_node_count;
extern AvlNode *centrex_avl;

#pragma pack(push, 1)
typedef struct {
    uint32_t block_number;
    uint8_t state_hash[32];
    uint32_t active_trunk_mask;
    uint32_t nonce;
    uint64_t session_key;
    uint32_t gas_allowance;
    bool is_committed;
    char unicode_desc[64];
    float synth_frequency;
    uint32_t checksum;
} BlueBoxBlockState;
#pragma pack(pop)

extern BlueBoxBlockState current_block_state;

// Blue Box / Tone Generation functions
void generate_sf_seizure(float *buffer, int count);
void generate_mf_kp(float *buffer, int count);
void generate_mf_st(float *buffer, int count);
void generate_mf_digit(char digit, float *buffer, int count);
void generate_red_box_coin(int coin_type, float *buffer, int count);

// Block management
void blue_box_init_block(uint32_t block_number, const uint8_t *initial_hash);
void blue_box_set_block_unicode_synth(const char *desc, float freq);
void blue_box_register_block_trunk(uint32_t trunk_id);
bool blue_box_commit_block(void);
BlueBoxBlockState blue_box_get_block_state(void);
bool blue_box_save_state_to_disk(const char *filepath);
bool blue_box_load_state_from_disk(const char *filepath);
bool blue_box_commit_and_persist_with_guard(const char *filepath, uint32_t expected_parent_block, const uint8_t *expected_parent_hash);

// 2-3 Tree Awareness
TwoThreeNode* blue_box_create_leaf(uint32_t key1, const char *val1, uint32_t key2, const char *val2);
TwoThreeNode* blue_box_create_internal(TwoThreeNode *c0, TwoThreeNode *c1, TwoThreeNode *c2);
void blue_box_bind_23_tree(TwoThreeNode *root);
const char* blue_box_retrieve_23_data(TwoThreeNode *node, uint32_t key);
bool blue_box_store_23_data(TwoThreeNode *node, uint32_t key, const char *new_value);
void blue_box_free_23_tree(TwoThreeNode *node);

// RDBMS & Index
void blue_box_rbt_insert(uint32_t block_number, TwoThreeNode *two_three_node);
TwoThreeNode* blue_box_rbt_lookup(uint32_t block_number);
uint32_t blue_box_query_blocks(const char *filepath, const char *field, const char *op, uint64_t value, uint32_t *results_out, uint32_t max_results);
bool blue_box_update_block_gas(const char *filepath, uint32_t block_number, uint32_t new_gas);
bool blue_box_delete_block(const char *filepath, uint32_t block_number);

// Transactional DML & WAL
void blue_box_begin_transaction(void);
bool blue_box_add_to_transaction(const BlueBoxBlockState *state);
bool blue_box_commit_transaction(const char *filepath);
void blue_box_rollback_transaction(void);
void blue_box_recover_wal(const char *filepath);

// Aggregation & Sorting
uint64_t blue_box_aggregate_blocks(const char *filepath, const char *field, const char *agg_func);
uint32_t blue_box_query_blocks_sorted(const char *filepath, const char *field, const char *op, uint64_t value, const char *sort_field, uint32_t *results_out, uint32_t max_results);

// Centrex & Dialing
bool blue_box_decode_access_code(const char *dial_sequence);
void blue_box_centrex_add_alias(uint32_t alias_code, uint32_t target_trunk);
uint32_t blue_box_centrex_resolve_route(uint32_t dial_code);
uint32_t blue_box_centrex_get_sorted_routes(uint32_t *keys_out, uint32_t *vals_out, uint32_t max_results);
void blue_box_centrex_add_unicode_alias(const char *unicode_name, uint32_t target_trunk);
uint32_t blue_box_centrex_resolve_unicode_route(const char *unicode_name);
void blue_box_centrex_add_unicode_forward(const char *src_name, const char *dest_name);

// ZMM Direct Dispatch & Citrix Remote
int blue_box_dispatch_zmm_rpc(const char *json_in, char *output_buf, size_t out_max);
size_t blue_box_citrix_compress_frame(const uint8_t *fb, size_t size, uint8_t *compressed_out, size_t max_out);
size_t blue_box_citrix_compress_audio(const float *samples, size_t count, uint8_t *compressed_out, size_t max_out);

uint32_t calculate_crc32(const uint8_t *data, size_t length);

bool generate_mf_tone(char digit, float *buffer, int num_samples);
int generate_red_box_coin_tone(int coin_type, float *buffer, int max_samples);
const char *blue_box_get_immutable_address(uint32_t trunk_id);
void blue_box_accumulate_state(uint64_t input_signal);
uint64_t blue_box_get_accumulator(void);
void blue_box_crypt_payload(uint8_t *payload, size_t length);
AvlNode* avl_insert(AvlNode *node, uint32_t val, uint32_t block_number);
bool blue_box_detect_and_accumulate(const float *samples, int num_samples);

void blue_box_centrex_set_trunk_rate(uint32_t trunk_id, uint32_t rate_per_min);
uint32_t blue_box_centrex_get_trunk_rate(uint32_t trunk_id);
void blue_box_accumulate_coin(int denomination);
bool blue_box_generate_erc20_tx(char *tx_buf, size_t max_len);
bool blue_box_deplete_session_gas(uint32_t trunk_id, uint32_t active_seconds);
bool blue_box_validate_slug(const float *samples, int num_samples);
void blue_box_rotate_key_on_coin(int denomination);
bool blue_box_negotiate_tariff(uint32_t trunk_id, uint32_t *rate_out);
bool blue_box_send_udp_tone(uint32_t port, const float *samples, size_t count);
bool blue_box_kermit_send_udp(uint32_t port, const uint8_t *packet, size_t len);
bool blue_box_send_udp_billing_alert(uint32_t port);
void blue_box_ouroboros_tick(void);
bool blue_box_verify_dual_stack(const uint8_t *sig, size_t sig_len, const uint8_t *pubkey, size_t pubkey_len, uint64_t max_pll_deviation);

#endif
