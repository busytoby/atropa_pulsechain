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

#endif // LIBANTIGRAVITY_INTEROP_H
