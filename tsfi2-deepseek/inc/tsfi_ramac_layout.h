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

// Translation Lookaside Buffer (TLB) entry structure for hardware lookup acceleration
typedef struct {
    uint32_t virtual_page;  // Virtual address segment/page boundary key
    uint32_t real_page;     // Mapped real memory page frame address
    int write_protect;      // Page-protection status bit cached from page tables
    int valid;              // Validity flag
} tsfi_s370_tlb_entry;

// System/370 Processor Status Word (PSW) Privilege & Security Mode Context
typedef struct {
    int supervisor_state; // Privilege mode (1: Supervisor State, 0: Problem State)
    int lap_enabled;      // Low-Address Protection (LAP) enabled flag (Control Reg 0 bit)
    uint8_t psw_key;      // Current execution access key (4-bit key, 0-15)
    tsfi_s370_psw current_psw; // Current CPU PSW state
    tsfi_s370_tlb_entry tlb[8]; // 8-entry direct mapped TLB cache simulation
} tsfi_s370_cpu_state;

// LAU Account PKI Key verified token context
typedef struct {
    char account_address[64];
    uint8_t public_key[32];
    int is_admin_tier; // Admin tier receives PSW master key 0 privileges
} tsfi_lau_account;

// Quadtree node partitioning analog coordinates, with values serialized in COMP-3 packed decimal format
typedef struct {
    double boundary_x;         // Center coordinates of the quadrant
    double boundary_y;
    double boundary_size;
    uint8_t packed_val[16];    // Value stored in COMP-3 packed decimal format
    int val_len;               // Length of packed decimal array
    int children_indices[4];   // Indirection indices of sub-quadrants (-1 if leaf)
    int is_active;
} tsfi_quadtree_node;

// Saburo Muroga parametron logic circuit node
typedef struct {
    int sources[3]; // Reference index: L >= 0 -> Node output L, -1 -> Constant 0, -2 -> Constant 1, L < -2 -> Input [-(L + 3)]
    int invert[3];  // Inversion flag for each input phase (1: Inverted, 0: Normal)
    int output;     // Phase state output cache (0 or 1)
} tsfi_parametron_node;

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
int tsfi_ramac_plugboard_route(const char *wiring, const uint8_t *src, uint8_t *dest, int max_len);

// Read-after-write verification loop (Double-Read verification)
int tsfi_ramac_write_verified(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder);

// Accumulator management
void tsfi_ramac_acc_init(tsfi_ramac_acc_model *model);
int tsfi_ramac_acc_add(tsfi_ramac_acc_model *model, int acc_id, int64_t val);
int tsfi_ramac_acc_div(tsfi_ramac_acc_model *model, int acc_id, int64_t val);

// IBM 370 printer-keyboard inquiry station console parser
int tsfi_ramac_inquiry_station(tsfi_ramac_record *disk, const char *command, char *response_out, int max_len);

// BCD 7-bit parity checker
int tsfi_ramac_check_parity(const char *str);

// IBM 305 Processor Loop
int tsfi_ramac_alu_exec(tsfi_ramac_acc_model *model, tsfi_ramac_instruction *program, int program_size);

// System/370 Dynamic Address Translation (DAT) translation lookup emulators
int tsfi_s370_dat_translate(uint32_t virtual_addr, 
                            tsfi_s370_segment_entry *seg_table, int seg_count,
                            tsfi_s370_page_entry *page_tables,
                            uint32_t *out_physical_addr, int *out_write_protected);

// System/370 Channel Command Word (CCW) I/O program chains...
int tsfi_s370_channel_execute(tsfi_ramac_record *disk, int total_slots,
                              tsfi_s370_ccw *ccw_chain, int chain_len,
                              uint8_t *memory_pool, int mem_size);

// System/370 Storage Key hardware protection checks
int tsfi_s370_check_storage_key(uint8_t psw_key, uint32_t real_addr, int is_write,
                                tsfi_s370_storage_key *block_keys, int block_count);

// System/370 Privilege & Security Mode Checks
int tsfi_s370_validate_instruction(tsfi_s370_cpu_state *cpu, const char *op_code);

// System/370 Write Validation (integrates Storage Keys, Low-Address Protection, and Virtual Page-Protection)
int tsfi_s370_validate_write(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                             int is_write_protected_page,
                             tsfi_s370_storage_key *block_keys, int block_count);

// System/370 LAU Account PKI Key Authorization
int tsfi_s370_authorize_psw_key(tsfi_lau_account *account, 
                                const uint8_t *signature, int sig_len,
                                const uint8_t *message, int msg_len,
                                uint8_t *out_psw_key);

// System/370 Security Hardware Program Interruption & PSW Swap handling
int tsfi_s370_trigger_program_interrupt(tsfi_s370_cpu_state *cpu, uint16_t pic,
                                        uint8_t *real_memory, int mem_size);

// System/370 COMP-3 Packed Decimal (COBOL style) Arithmetic Unit emulators
int tsfi_s370_pack(const char *zoned_str, uint8_t *packed_out, int max_len);
int tsfi_s370_unpack(const uint8_t *packed, int packed_len, char *zoned_out, int max_len);
int tsfi_s370_packed_add(const uint8_t *a, int a_len,
                         const uint8_t *b, int b_len,
                         uint8_t *dest_out, int dest_max_len);

// System/370 Supervisor Call (SVC) security interruption gateway
int tsfi_s370_trigger_svc(tsfi_s370_cpu_state *cpu, uint8_t svc_code,
                          uint8_t *real_memory, int mem_size);

// Benson-Lehner style Data Reduction Unit
int tsfi_s370_data_reduction_unit(double x, double y, double scale,
                                  uint8_t *dest_out, int dest_max_len);

// Quadtree serialization to disk using the strict Rule 13 .dat.bin format
int tsfi_s370_serialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int node_count);
int tsfi_s370_deserialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int max_nodes);

// Translates virtual address using CPU TLB cache or falls back to table walk (populating TLB)
int tsfi_s370_dat_translate_with_tlb(tsfi_s370_cpu_state *cpu, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint32_t *out_physical_addr, int *out_write_protected);

// Purges/Invalidates all entries in the CPU TLB cache
void tsfi_s370_tlb_purge(tsfi_s370_cpu_state *cpu);

// Benson-Lehner OSCAR (Oscillograph Analyzer and Reader) style non-linear calibration map converter
int tsfi_s370_oscar_reader(double analog_amplitude, const double *calibration_table, int table_size,
                           uint8_t *dest_out, int dest_max_len);

// Alfred M. Freudenthal viscoelastic soft-body physics solver applied strictly to FET discharge cycles
int tsfi_s370_fet_discharge_freudenthal(double initial_charge, double time_step, double mass,
                                        double spring_k, double damping_c, int steps, double *out_decay_charges);

// Alfred M. Freudenthal fatigue damage accumulation solver applied strictly to FET gate dielectric discharge stress
double tsfi_s370_fet_gate_fatigue_freudenthal(const double *stress_amplitudes, int cycle_count,
                                              double reference_stress, double shape_parameter);

// Alfred M. Freudenthal structural reliability safety solver applied strictly to FET gate oxide breakdown
int tsfi_s370_fet_reliability_freudenthal(double mean_resistance, double std_resistance,
                                          double mean_stress, double std_stress,
                                          double *out_beta, double *out_pf);

// Ora C. Roehl (Keystone Custodian Funds VP of Research) style stochastic portfolio yield strategy optimizer
int tsfi_s370_portfolio_strategy_keystone(const double *asset_yields, const double *weights, int asset_count,
                                           double *out_expected_return, double *out_variance);

// Luis J. A. Villalon style executive techniques decision-tree evaluator
int tsfi_s370_executive_decision_villalon(int decision_count, const double *benefit, const double *cost,
                                          const double *risk_prob, double *out_expected_net_value,
                                          int *out_optimal_decision_idx);

// Maurice Nelles (Borg-Warner Director of Research) style deliberate creativeness heuristic mutator
int tsfi_s370_deliberate_creativeness_nelles(double *parameters, int count, unsigned int seed);

// Vladimir Zworykin (RCA) style developmental project scale divergence predictor
int tsfi_s370_project_scale_zworykin(double initial_budget, double initial_months,
                                      double *out_actual_budget, double *out_actual_months);

// Z-machine RAMAC storage mapping memory access routines (Project Peters realization)
int tsfi_s370_zmachine_read_byte(const tsfi_ramac_record *disk, uint32_t zmachine_addr, uint8_t *out_val);
int tsfi_s370_zmachine_write_byte(tsfi_ramac_record *disk, uint32_t zmachine_addr, uint8_t val);

// System/370 Dynamic Address Translation (DAT) logical mapping to RAMAC CHS coordinates
int tsfi_s370_dat_ramac_translate(uint32_t virtual_addr, 
                                  tsfi_s370_segment_entry *seg_table, int seg_count,
                                  tsfi_s370_page_entry *page_tables,
                                  tsfi_ramac_chs *out_chs);

// SCSI WinchesterMQ Handshake Loop emulation interface (Rule 5 Auncient routing compliant)
int tsfi_s370_winchester_mq_handshake(uint8_t *scsi_bus_status, uint8_t *data_reg,
                                      const uint8_t *stream, int stream_len,
                                      uint8_t *out_buffer, int max_out_len);

// Benson-Lehner OSCAR reader with polynomial calibration curves
int tsfi_s370_oscar_reader_polynomial(double analog_amplitude, const double *coefficients, int coeff_count,
                                      uint8_t *dest_out, int dest_max_len);

// Translates a field of a punched card directly into a COMP-3 packed BCD structure
int tsfi_s370_punched_card_to_comp3(const tsfi_ramac_card *card, int start_col, int end_col,
                                    uint8_t *packed_out, int max_len);

// Streams data via WinchesterMQ loop and commits as records to RAMAC cylinders
int tsfi_s370_scsi_stream_to_ramac(tsfi_ramac_record *disk, uint8_t *scsi_status, uint8_t *data_reg,
                                    const uint8_t *stream, int stream_len, int target_cylinder);

// Checks Benson-Lehner OSCAR reader analog output value decay against the FET viscoelastic discharge boundaries
int tsfi_s370_oscar_soft_body_validate(double analog_val, double mass, double spring_k, double damping_c,
                                       double *out_decay_charges, int steps);

// SAGE AN/FSQ-7 style active-passive dual-CPU redundancy and failover state controller
int tsfi_s370_sage_redundancy_monitor(int cpu_a_status, int cpu_b_status, int *active_cpu);

// Engelbart & Bourne technical information search index resolver
int tsfi_s370_engelbart_index_resolve(const char *abstract, const char **keywords, int keyword_count,
                                      uint8_t *comp3_out, int max_len);

// Saburo Muroga style parametron majority phase decision logic gate emulator
int tsfi_s370_muroga_parametron_majority(int phase_in_1, int phase_in_2, int phase_in_3, int *phase_out);

// Dynamic Parametron Circuit Netlist Evaluator
int tsfi_s370_parametron_circuit_eval(tsfi_parametron_node *nodes, int node_count,
                                       const int *inputs, int input_count);

// Engelbart & Bourne optical Batten/Peek-a-boo card-matching simulator
int tsfi_s370_peek_a_boo_card_match(const uint32_t *card_a, const uint32_t *card_b,
                                     uint32_t *out_matching, int word_count);

// Saburo Muroga style threshold logic gate emulator
int tsfi_s370_muroga_threshold_gate(const int *inputs, const int *weights, int input_count,
                                    int threshold, int *output);

// Autonetics Recomp II 40-bit word instruction decoder
int tsfi_s370_recomp_ii_decode_word(uint64_t raw_word, int *op1, int *addr1, int *op2, int *addr2);

// Autonetics Recomp II drum memory rotational sector scheduler (minimum latency coding)
int tsfi_s370_recomp_ii_drum_schedule(int current_sector, int execution_cycles, int *out_optimal_sector);

// Perforated paper tape synthesizer sequencer instrument
int tsfi_s370_paper_tape_synthesizer(const uint8_t *tape_data, int length, int channels,
                                     double *out_audio, int max_samples, double sample_rate);

#endif // TSFI_RAMAC_LAYOUT_H
