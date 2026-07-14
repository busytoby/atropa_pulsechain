#ifndef LIBANTIGRAVITY_INTEROP_H
#define LIBANTIGRAVITY_INTEROP_H

#include <stdint.h>

// Auncient VM WinchesterMQ State probe
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t contour;
    uint64_t pole;
    uint64_t coordinate;
    uint64_t foundation;
    uint64_t element;
    uint64_t dynamo;
    uint64_t monopole;
    uint64_t barn;
} AuncientRegisterState;

// WinchesterMQ Queue/LUN state structure
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t pending_ack;
    uint8_t sectors[16][256]; // 16 sector capacity for flow control backpressure
} InteropLUN;

// Thread-safe interop registry sharing VM and WMQ queue states with the renderer
typedef struct {
    AuncientRegisterState active_state;
    uint32_t current_epoch;
    uint32_t active_cycles;
    float frame_modulation_factor;
    float verlet_x[16];
    float verlet_y[16];
    float verlet_z[16];
    uint32_t active_verlet_count;
    uint32_t boundary_collision_flags;
    InteropLUN network_lun; // Maps to LUN 4 (Network queue) for live chat telemetry
} InteropRegistry;

#include <stddef.h>

// Static Table representation
typedef struct {
    uint64_t keys[10];
    uint64_t values[10];
    uint32_t count;
} InteropStaticTable;

typedef struct InteropBSTNode {
    uint64_t key;
    uint32_t row_index;
    struct InteropBSTNode *left;
    struct InteropBSTNode *right;
} InteropBSTNode;

// Dynamic Table RDBMS representation for Agentic Memory
typedef struct {
    uint32_t active;
    uint32_t col_count;
    uint64_t *rows; // RCU-swapped pointer
    uint32_t count;
    uint32_t capacity;
    InteropBSTNode *bst_root; // BST index root pointer
} InteropDynamicTable;

// Verifiable Transaction Audit Log Entry
typedef struct {
    uint64_t prev_hash;  // Hash of the previous entry
    uint32_t selector;   // Selector called
    uint32_t arg_count;  // Count of args
    uint64_t args[5];    // Shifted args array
    uint64_t entry_hash; // Current block signature
} InteropAuditEntry;

// Coaxial Shared Memory Table Layout (Zero-Copy Relative Pointer Map)
typedef struct {
    volatile int lock;             // Atomic spinlock
    volatile uint32_t rows_offset; // Relative offset to rows array from header start
    volatile uint32_t count;       // Current count of records
    volatile uint32_t capacity;    // Maximum records capacity
    volatile uint32_t col_count;   // Columns per record
} InteropCoaxialTable;

void interop_coaxial_init_table(InteropCoaxialTable *table, uint32_t capacity, uint32_t col_count);
uint64_t interop_coaxial_insert(InteropCoaxialTable *table, const uint64_t *args, size_t arg_count);
uint64_t interop_coaxial_select(const InteropCoaxialTable *table, uint64_t key);

// System-wide Coaxial Shared Memory Registry Layout
typedef struct {
    InteropCoaxialTable trie_route_table;       // Merkle-Patricia Trie Routing
    InteropCoaxialTable wmq_event_table;        // WinchesterMQ Event Telemetry
    InteropCoaxialTable gas_calibration_table;  // Dynamic Gas/Fee Calibration
    InteropCoaxialTable ipc_signal_table;       // Inter-Process Signal Routing
    InteropCoaxialTable vram_page_table;        // VRAM Page-Frame Allocations
    InteropCoaxialTable jit_reflection_table;   // JIT Reflection Method Dispatch
    InteropCoaxialTable peer_registry_table;    // Peer-to-Peer Agent Routing
    InteropCoaxialTable radio_packet_table;     // LoRa/OOK Radio Transmission Queue
    InteropCoaxialTable cpu_snapshot_table;     // Live CPU Snapshot debugging registers
} InteropSystemLedger;

// Interop API exports
void interop_registry_init(InteropRegistry *reg);
void interop_registry_update(InteropRegistry *reg, const AuncientRegisterState *state, uint32_t epoch);
void interop_registry_probe(const InteropRegistry *reg, AuncientRegisterState *out_state, uint32_t *out_epoch);
void interop_system_ledger_init(InteropSystemLedger *ledger);

// WinchesterMQ queue interfaces
void interop_mq_put(InteropLUN *lun, const uint8_t *data);
int interop_mq_get_phase1(InteropLUN *lun, uint8_t *out_data);
int interop_mq_ack_phase2(InteropLUN *lun);

// Thread-safe dynamic database operations
uint64_t interop_agent_create_table(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_insert_dynamic(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_select_dynamic(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_rdbms_dispatch(void *payload, const uint64_t *args, size_t arg_count);
void interop_agent_destroy_table(InteropDynamicTable *table);
uint64_t interop_agent_log_transaction(void *payload, uint32_t selector, const uint64_t *args, size_t arg_count);
int interop_agent_verify_audit_log(void *payload, uint64_t *out_final_hash);

typedef struct ABIDispatchMap ABIDispatchMap;
int interop_mq_dispatch_rdbms(InteropLUN *lun, void *db_payload, ABIDispatchMap *map);

// Thread-safe static database operations
uint64_t interop_agent_insert(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_query(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_insert_external(void *my_payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_delete(void *payload, const uint64_t *args, size_t arg_count);
uint64_t interop_agent_update(void *payload, const uint64_t *args, size_t arg_count);

int interop_coaxial_bridge_init(const char *socket_path);
int interop_coaxial_bridge_poll(int server_fd, InteropCoaxialTable *table);

// RDBMS-PLL state synchronizer definitions
typedef struct {
    volatile uint32_t host_state_counter;
    volatile uint32_t guest_state_counter;
    volatile uint32_t phase_error;
    volatile uint32_t locked;
} InteropPLLHeader;

void interop_pll_init(InteropPLLHeader *pll);
int interop_pll_detect_phase_error(InteropPLLHeader *pll, uint32_t host_counter, uint32_t guest_counter);
int interop_pll_align_state(InteropPLLHeader *pll, InteropCoaxialTable *table, uint32_t target_offset);

int interop_kermit_send_packet(InteropCoaxialTable *table, InteropPLLHeader *pll, uint32_t packet_num, const uint8_t *data, size_t size);
int interop_kermit_receive_packet(const InteropCoaxialTable *table, InteropPLLHeader *pll, uint32_t packet_num, uint8_t *out_data, size_t *out_size);

// Universal Turing Machine over Coaxial RDBMS-PLL definitions
typedef struct {
    uint32_t current_state;
    int32_t head_index;
    uint32_t halted;
} InteropTuringState;

void interop_turing_init(InteropTuringState *turing);
int interop_turing_run_step(InteropTuringState *turing, InteropCoaxialTable *tape_table, const InteropCoaxialTable *rules_table);

// Lock-Free Coaxial Ring Queue definitions
typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    uint32_t capacity;
    uint32_t item_size;
    uint32_t data_offset;
} InteropCoaxialQueue;

void interop_queue_init(InteropCoaxialQueue *q, uint32_t capacity, uint32_t item_size);
int interop_queue_push(InteropCoaxialQueue *q, const void *item);
int interop_queue_pop(InteropCoaxialQueue *q, void *out_item);

// Bitcoin-style Covenant state evolution definitions
typedef struct {
    uint64_t prev_state_hash;
    uint64_t next_state_hash;
    uint64_t tx_input_witness;
} InteropCovenantState;

void interop_covenant_init(InteropCovenantState *cov, uint64_t prev_hash);
int interop_covenant_verify_evolution(InteropCovenantState *cov, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules, uint64_t expected_next_hash);

uint64_t fnv1a_hash_cascade(uint64_t initial_hash, const void *data, size_t len);
int interop_covenant_deploy_yul(InteropCoaxialTable *rules_table, const uint64_t *yul_rules, size_t rule_count);

// L2 optimizations: Rollups, Fraud Proofs, and SIMD hashing
typedef struct {
    uint64_t start_state_hash;
    uint64_t end_state_hash;
    uint32_t step_count;
} InteropRollupBatch;

int interop_covenant_verify_batch(InteropRollupBatch *batch, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules, uint64_t expected_end_hash);
int interop_covenant_prove_fraud(uint64_t disputed_prev_hash, uint64_t asserted_next_hash, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules);
uint64_t fnv1a_hash_vectorized(uint64_t initial_hash, const void *data, size_t len);

typedef struct {
    uint32_t exit_code;
    uint32_t padding;
    uint64_t return_value;
    uint64_t next_state_hash;
} InteropTuringResponse;

int interop_covenant_verify_response(const InteropTuringResponse *response, uint64_t expected_hash, uint32_t expected_exit_code, uint64_t expected_return);

typedef struct {
    uint64_t cell_index;
    uint64_t cell_value;
} InteropStateDelta;

int interop_covenant_replay_log(InteropCoaxialTable *tape, const InteropStateDelta *deltas, size_t delta_count, uint64_t expected_hash);

typedef struct {
    uint32_t scheduled_epoch;
    uint32_t scheduled_selector;
    uint64_t scheduled_arg;
    int is_active;
} InteropAgentScheduler;

int interop_scheduler_register(InteropAgentScheduler *sched, uint32_t epoch, uint32_t selector, uint64_t arg);
int interop_scheduler_tick(InteropAgentScheduler *sched, uint32_t current_epoch, uint64_t *triggered_val);

uint64_t interop_mamt_adduct(uint64_t child, uint64_t secret);
int interop_mamt_verify(uint64_t child, uint64_t parent, uint64_t secret);
int interop_scheduler_tick_multilane(InteropAgentScheduler *scheds, size_t count, uint32_t current_epoch, uint64_t *triggered_vals);
void interop_gas_calibrate(uint32_t cache_misses, uint32_t *gas_price);

typedef struct {
    uint64_t target_agent_addr;
    uint64_t preference_weight;
} InteropPreferenceEntry;

uint64_t interop_fee_calculate(uint32_t poly_degree, uint32_t cell_count, uint32_t gas_price);
int interop_preference_accumulate(InteropPreferenceEntry *entries, size_t count, uint64_t target, uint64_t weight);

int interop_reentrancy_lock(uint32_t *bitmap, uint32_t depth);
void interop_reentrancy_unlock(uint32_t *bitmap, uint32_t depth);
int interop_scheduler_tick_prioritized(InteropAgentScheduler *scheds, size_t count, uint32_t current_epoch, uint64_t *triggered_vals, const uint32_t *priorities);
int interop_sdsa_verify_alignment(const void *data);

typedef struct {
    uint64_t threshold;
    uint32_t left_child_idx;
    uint32_t right_child_idx;
    uint32_t branch_selector;
} InteropDecisionNode;

uint32_t interop_decision_tree_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t accumulator_val);

typedef struct {
    uint64_t agent_addr;
    uint64_t coord[3];
} InteropKNNAgent;

uint64_t interop_knn_distance(const uint64_t *coord1, const uint64_t *coord2);
int interop_knn_search(const InteropKNNAgent *agents, size_t count, const uint64_t *query_coord, uint64_t *out_neighbors, size_t k);
int interop_coaxial_cluster(const uint64_t *coords, size_t count, uint64_t *centroids, size_t k, uint32_t *assign);
int interop_mamt_cluster(const uint64_t *hashes, size_t count, uint32_t *lanes, size_t k);
int interop_ac_cache_cluster(const uint32_t *freqs, size_t count, uint32_t *lines, size_t k);
int interop_preference_cluster(const uint64_t *prefs, size_t count, uint32_t *shards, size_t k);

int interop_mamt_knn_search(const uint64_t *leaves, size_t count, const uint64_t *query_hash, uint64_t *out_leaves, size_t k);
int interop_ac_cache_knn_search(const uint32_t *freqs, size_t count, uint32_t query_freq, uint32_t *out_freqs, size_t k);
int interop_preference_knn_search(const uint64_t *prefs, size_t count, uint64_t query_pref, uint64_t *out_prefs, size_t k);

uint32_t interop_mamt_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t hash_val);
uint32_t interop_ac_cache_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t frequency);
uint32_t interop_fee_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t complexity);

typedef struct {
    uint32_t thresholds[3];
    uint32_t children[4];
} InteropMultiDecisionNode;

uint32_t interop_multi_decision_evaluate(const InteropMultiDecisionNode *nodes, uint32_t root_idx, uint64_t accumulator_val);
uint64_t interop_knn_distance_avx512(const uint64_t *coord1, const uint64_t *coord2);
void interop_multi_decision_evaluate_avx512(const InteropMultiDecisionNode *nodes, uint32_t root_idx, const uint64_t *acc_vals, uint32_t *out_results, size_t count);
int interop_coaxial_cluster_weighted(const uint64_t *coords, const uint64_t *weights, size_t count, uint64_t *centroids, size_t k, uint32_t *assign);
int interop_lsh_ann_search(const InteropKNNAgent *agents, size_t count, const uint64_t *query_coord, uint64_t *out_neighbors, size_t k);
void interop_lsh_project_avx512(const InteropKNNAgent *agents, size_t count, uint64_t *out_lsh);
void interop_multi_decision_evaluate_sorted(const InteropMultiDecisionNode *nodes, uint32_t root_idx, uint64_t *acc_vals, uint32_t *out_results, size_t count);
int interop_coaxial_cluster_hierarchical(const uint64_t *coords, size_t count, uint64_t *centroids, size_t k, uint32_t *assign);
uint32_t interop_decision_vector_search_avx512(const uint32_t *thresholds, size_t count, uint32_t query_val);
uint64_t interop_knn_distance_minkowski(const uint64_t *coord1, const uint64_t *coord2, uint32_t p);
int interop_coaxial_cluster_adaptive(const uint64_t *coords, size_t count, uint64_t *centroids, uint32_t *k_io, uint32_t *assign);

typedef struct {
    uint32_t x_min, y_min, x_max, y_max;
    uint32_t value;
    uint32_t children[4];
} InteropQuadNode;

int interop_quadtree_write(const char *filepath, const InteropQuadNode *nodes, size_t count);
int interop_quadtree_read(const char *filepath, InteropQuadNode *nodes_out, size_t max_nodes);
uint32_t interop_quadtree_query(const InteropQuadNode *nodes, uint32_t root_idx, uint32_t x, uint32_t y);
int interop_quadtree_quadrant_check_avx512(const uint32_t *x_mins, const uint32_t *x_maxs, const uint32_t *y_mins, const uint32_t *y_maxs, uint32_t x, uint32_t y);
int interop_quadtree_write_rle(const char *filepath, const InteropQuadNode *nodes, size_t count);
int interop_quadtree_read_rle(const char *filepath, InteropQuadNode *nodes_out, size_t max_nodes);
void interop_quadtree_veb_align(const InteropQuadNode *src, InteropQuadNode *dst, size_t count);
void interop_rle_decode_avx512(const uint32_t *runs, const uint32_t *values, size_t count, uint32_t *out);
void interop_multi_decision_prune(InteropMultiDecisionNode *nodes, size_t count);
int interop_coaxial_cluster_minkowski(const uint64_t *coords, size_t count, uint64_t *centroids, size_t k, uint32_t *assign, uint32_t p);

#endif // LIBANTIGRAVITY_INTEROP_H
