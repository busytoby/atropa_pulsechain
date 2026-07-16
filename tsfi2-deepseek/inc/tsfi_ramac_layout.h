#ifndef TSFI_RAMAC_LAYOUT_H
#define TSFI_RAMAC_LAYOUT_H

#include "tsfi_dat.h"
#include <stdint.h>

// IBM 305 RAMAC disk geometry parameters
#define RAMAC_CYLINDERS 100
#define RAMAC_HEADS      50  // 50 disks, 1 head per surface
#define RAMAC_SECTORS    20  // 20 sectors per track
#define RAMAC_WORDS      8   // 8 words (32 bytes) per sector

typedef struct {
    int cylinder;
    int head;
    int sector;
    int word_offset;
} tsfi_ramac_chs;

// RAMAC-style cylinder overflow hash record layout
typedef struct {
    char key[32];
    char value[32];
    int next_overflow_index; // Index of next record in case of collision
    int is_active;
} tsfi_ramac_record;

// Non-preferential accumulator model (Rule 12 compliant)
typedef struct {
    int64_t accumulators[10]; // ACC0 to ACC9
    int64_t isolation_trap;   // Isolated state for mathematical discontinuities
    int trap_active;
} tsfi_ramac_acc_model;

// IBM 380 80-column punched card structure
typedef struct {
    char columns[80];
} tsfi_ramac_card;

// IBM 305 ALU program instruction step
typedef struct {
    char op[8];   // "ADD", "SUB", "DIV", "CMP", "JEQ"
    int acc_dest; // Destination accumulator (0..9)
    int acc_src;  // Source accumulator (0..9) or constant value
    int constant; // Flag indicating if constant value is used
    char label[16]; // Label for jumps
} tsfi_ramac_instruction;

// System/370 Dynamic Address Translation (DAT) structures
typedef struct {
    uint32_t page_table_origin; // Address of page table
    uint32_t length;            // Segment length
    int invalid;                // Segment invalid bit (I)
} tsfi_s370_segment_entry;

typedef struct {
    uint32_t page_frame_real_addr; // Physical real address page offset
    int invalid;                   // Page invalid bit (I)
    int write_protect;             // Page-Protection bit (P) - read-only virtual page
} tsfi_s370_page_entry;

// System/370 Channel Command Word (CCW) structure
typedef struct {
    uint8_t cmd_code;   // Command code (e.g. 0x01: write, 0x02: read, 0x07: seek)
    uint32_t data_addr; // Target memory data address pointer
    uint8_t flags;      // CCW flags (bit 1: Chain Data, bit 2: Chain Command)
    uint16_t count;     // Transfer byte count
} tsfi_s370_ccw;

// System/370 Storage Key (7-bit hardware protection key per 2KB/4KB block)
typedef struct {
    uint8_t acc;           // Access control bits (4 bits, 0-15)
    uint8_t fetch_protect; // Fetch protection bit (F)
    uint8_t referenced;    // Reference bit (R)
    uint8_t changed;       // Change bit (C)
} tsfi_s370_storage_key;

// System/370 Program Status Word (PSW) layout
typedef struct {
    uint8_t key;                  // Access key (4 bits)
    int problem_state;            // Problem state flag
    uint32_t instruction_address; // Instruction address pointer (31-bit)
} tsfi_s370_psw;

// System/370 Processor Status Word (PSW) Privilege & Security Mode Context
typedef struct {
    int supervisor_state; // Privilege mode (1: Supervisor State, 0: Problem State)
    int lap_enabled;      // Low-Address Protection (LAP) enabled flag (Control Reg 0 bit)
    uint8_t psw_key;      // Current execution access key (4-bit key, 0-15)
    tsfi_s370_psw current_psw; // Current CPU PSW state
} tsfi_s370_cpu_state;

// LAU Account PKI Key verified token context
typedef struct {
    char account_address[64];
    uint8_t public_key[32];
    int is_admin_tier; // Admin tier receives PSW master key 0 privileges
} tsfi_lau_account;

// Translates a flat index to CHS coordinates
tsfi_ramac_chs tsfi_ramac_index_to_chs(int index);

// Translates CHS coordinates back to a flat index
int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs);

// Calculates the estimated seek latency (in microseconds) between two flat indices
double tsfi_ramac_calculate_seek(int from_index, int to_index);

// Optimizes and serializes a Double-Array Trie (DAT) to disk using cylindrical layout
int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath);

// Hashes a key to a primary sector index within a given cylinder
int tsfi_ramac_hash_key(const char *key, int cylinder);

// Inserts a record into the cylinder-overflow disk layout, keeping overflows within the same cylinder
int tsfi_ramac_insert_record(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder, double *out_total_seek_us);

// Searches a record in the cylinder-overflow layout
const char* tsfi_ramac_search_record(tsfi_ramac_record *disk, const char *key, int cylinder, double *out_total_seek_us);

// IBM 305 RAMAC plugboard wiring control panel emulator
// Parses wiring rules like "0..7->8..15" and routes source buffer sections to destination
int tsfi_ramac_plugboard_route(const char *wiring, const uint8_t *src, uint8_t *dest, int max_len);

// Read-after-write verification loop (Double-Read verification)
// Returns 0 if verified, -1 if parity or content mismatch occurs
int tsfi_ramac_write_verified(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder);

// Accumulator management
void tsfi_ramac_acc_init(tsfi_ramac_acc_model *model);
int tsfi_ramac_acc_add(tsfi_ramac_acc_model *model, int acc_id, int64_t val);
int tsfi_ramac_acc_div(tsfi_ramac_acc_model *model, int acc_id, int64_t val);

// IBM 370 printer-keyboard inquiry station console parser
int tsfi_ramac_inquiry_station(tsfi_ramac_record *disk, const char *command, char *response_out, int max_len);

// BCD 7-bit parity checker
// Returns 1 if all characters in the string have valid odd parity (even number of 1s plus parity bit = odd)
int tsfi_ramac_check_parity(const char *str);

// IBM 305 Processor Loop
// Executes a plugboard ALU instruction list on the accumulator model
int tsfi_ramac_alu_exec(tsfi_ramac_acc_model *model, tsfi_ramac_instruction *program, int program_size);

// System/370 Dynamic Address Translation (DAT) translation lookup emulators
// Returns 0 on success, -1 on translation exceptions (segment invalid/page invalid)
// out_write_protected is populated with the Page-Protection bit (P)
int tsfi_s370_dat_translate(uint32_t virtual_addr, 
                            tsfi_s370_segment_entry *seg_table, int seg_count,
                            tsfi_s370_page_entry *page_tables,
                            uint32_t *out_physical_addr, int *out_write_protected);

// System/370 Channel I/O Program execution emulator
int tsfi_s370_channel_execute(tsfi_ramac_record *disk, int total_slots,
                              tsfi_s370_ccw *ccw_chain, int chain_len,
                              uint8_t *memory_pool, int mem_size);

// System/370 Storage Key hardware protection checks
// Returns 0 if access is permitted, -1 if protection exception occurs
int tsfi_s370_check_storage_key(uint8_t psw_key, uint32_t real_addr, int is_write,
                                tsfi_s370_storage_key *block_keys, int block_count);

// System/370 Privilege & Security Mode Checks
int tsfi_s370_validate_instruction(tsfi_s370_cpu_state *cpu, const char *op_code);

// System/370 Write Validation (integrates Storage Keys & Low-Address Protection)
int tsfi_s370_validate_write(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                             tsfi_s370_storage_key *block_keys, int block_count);

// System/370 LAU Account PKI Key Authorization
int tsfi_s370_authorize_psw_key(tsfi_lau_account *account, 
                                const uint8_t *signature, int sig_len,
                                const uint8_t *message, int msg_len,
                                uint8_t *out_psw_key);

// System/370 Security Hardware Program Interruption & PSW Swap handling
int tsfi_s370_trigger_program_interrupt(tsfi_s370_cpu_state *cpu, uint16_t pic,
                                        uint8_t *real_memory, int mem_size);

#endif // TSFI_RAMAC_LAYOUT_H
