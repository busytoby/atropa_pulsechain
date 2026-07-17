#ifndef TSFI_MAINFRAME_FIPS_H
#define TSFI_MAINFRAME_FIPS_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_ramac_layout.h"

// Scenario 142: IBM 3848 Cryptographic Subsystem Simulation
typedef struct {
    uint64_t master_key;
    int is_key_loaded;
} tsfi_crypto_subsystem;

void tsfi_crypto_init(tsfi_crypto_subsystem *crypto);
int tsfi_crypto_load_master_key(tsfi_crypto_subsystem *crypto, uint64_t master_key);
int tsfi_crypto_encrypt(tsfi_crypto_subsystem *crypto, const uint8_t *plain, uint8_t *cipher, int supervisor_state);
int tsfi_crypto_decrypt(tsfi_crypto_subsystem *crypto, const uint8_t *cipher, uint8_t *plain, int supervisor_state);

// Scenario 143: NBS FIPS PUB 60 Standard I/O Channel Interface
typedef struct tsfi_fips60_interface {
    uint8_t bus_out_command;
    uint8_t bus_in_status;
    int command_pending;
    int status_updated;
} tsfi_fips60_interface;

void tsfi_fips60_init(tsfi_fips60_interface *fips);
int tsfi_fips60_bus_out(tsfi_fips60_interface *fips, uint8_t command_byte, const uint8_t *data, uint16_t data_len);
int tsfi_fips60_status_in(const tsfi_fips60_interface *fips, uint8_t *out_status);

// Scenario 144: NBS FIPS PUB 54 Computer Output Microform (COM) Spool Formatter
#define MAX_COM_INDEX_ENTRIES 32
typedef struct {
    char key[16];
    int frame_number;
} tsfi_com_index_entry;

typedef struct {
    int current_frame;
    int line_count;
    tsfi_com_index_entry index_table[MAX_COM_INDEX_ENTRIES];
    int index_count;
} tsfi_com_formatter;

void tsfi_com_init(tsfi_com_formatter *fmt);
int tsfi_com_format_record(tsfi_com_formatter *fmt, const char *record_text, uint8_t *out_frame, uint16_t *out_len);
int tsfi_com_generate_index_frame(const tsfi_com_formatter *fmt, char *out_index_data, int max_len);

// Scenario 145: NBS FIPS PUB 48 Personal Identification Token Authenticator
#define MAX_FIPS48_BADGES 16
typedef struct {
    char user_id[16];
    uint32_t badge_id;
    uint16_t pin;
    int is_active;
} tsfi_fips48_badge;

typedef struct {
    tsfi_fips48_badge badges[MAX_FIPS48_BADGES];
    int badge_count;
    int failed_attempts;
    int successful_attempts;
} tsfi_fips48_authenticator;

void tsfi_fips48_init(tsfi_fips48_authenticator *auth);
int tsfi_fips48_register_badge(tsfi_fips48_authenticator *auth, const char *user_id, uint32_t badge_id, uint16_t pin);
int tsfi_fips48_register_lau_badge(tsfi_fips48_authenticator *auth, const char *user_id, uint32_t lau_token_address, uint16_t pki_key_pin);
int tsfi_fips48_authenticate(tsfi_fips48_authenticator *auth, uint32_t badge_id, uint16_t pin, int *out_status);

// Scenario 146: NBS FIPS PUB 62 Magnetic Tape Subsystem Interface
typedef struct {
    int block_position;
    int is_bot;
    int is_eot;
    int tape_mark_detected;
} tsfi_fips62_tape;

void tsfi_fips62_tape_init(tsfi_fips62_tape *tape);
int tsfi_fips62_tape_command(tsfi_fips62_tape *tape, uint8_t cmd_code, int *out_block_pos);
int tsfi_fips62_tape_read_to_virtual(tsfi_fips62_tape *tape, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint8_t *memory_pool, int mem_size,
                                     const uint8_t *tape_data, uint16_t data_len);

// Scenario 147: NBS FIPS PUB 63 Rotating Mass Storage Subsystem Interface
typedef struct {
    uint32_t current_cylinder;
    uint32_t current_sector;
    int is_ready;
    int recalibrate_requested;
} tsfi_fips63_disk;

void tsfi_fips63_disk_init(tsfi_fips63_disk *disk);
int tsfi_fips63_disk_command(tsfi_fips63_disk *disk, uint8_t cmd_code, uint32_t cylinder, uint32_t sector, uint8_t *out_status);

// Scenario 148: NBS FIPS PUB 68 Minimal BASIC Interpreter
typedef struct {
    int variables[26]; // A to Z variables
    int last_executed_line;
} tsfi_fips68_basic;

void tsfi_fips68_basic_init(tsfi_fips68_basic *basic);
int tsfi_fips68_basic_run(tsfi_fips68_basic *basic, const char *source_code, char *out_buffer, int max_len);

// Scenario 149: NBS FIPS PUB 69 Numeric Value Parser/Formatter
int tsfi_fips69_parse_numeric(const char *numeric_str, double *out_val);
int tsfi_fips69_format_numeric(double val, char *out_str, int max_len);

// Scenario 150: NBS FIPS PUB 79 Magnetic Tape Label Parser/Validator
int tsfi_fips79_parse_label(const uint8_t *label_block, char *out_file_id, uint32_t *out_serial, int *out_block_count);
int tsfi_fips79_format_label(uint8_t *out_block, const char *file_id, uint32_t serial, int block_count, const char *label_type);

// Scenario 151: NBS FIPS PUB 81 Block Cipher Modes of Operation (ECB/CBC)
int tsfi_fips81_encrypt_cbc(tsfi_crypto_subsystem *crypto, const uint8_t *plain, uint8_t *cipher, int blocks, const uint8_t *iv, int supervisor_state);
int tsfi_fips81_decrypt_cbc(tsfi_crypto_subsystem *crypto, const uint8_t *cipher, uint8_t *plain, int blocks, const uint8_t *iv, int supervisor_state);

// Scenario 152: NBS FIPS PUB 94 Power Disturbance Monitor
typedef struct {
    int total_events;
    int sag_count;
    int surge_count;
    int transient_count;
    int unsafe_power_state;
} tsfi_fips94_monitor;

void tsfi_fips94_power_init(tsfi_fips94_monitor *mon);
int tsfi_fips94_audit_voltage(tsfi_fips94_monitor *mon, double nominal_voltage, double actual_voltage, int *out_fault_type);

// Scenario 153: NBS FIPS PUB 73 Application Processing Controls Audit
typedef struct {
    int parsed_transactions;
    int validation_failures;
    int valid_transactions;
} tsfi_fips73_auditor;

void tsfi_fips73_audit_init(tsfi_fips73_auditor *auditor);
int tsfi_fips73_audit_transaction(tsfi_fips73_auditor *auditor, const char *record_name, int payload_val);

// Scenario 154: NBS FIPS PUB 38 Automated Documentation Compliance Auditor
int tsfi_fips38_audit_document(const char *doc_content, int *out_completeness_percent);

// Scenario 155: NBS FIPS PUB 1-1 Character Set Translation (ASCII / EBCDIC)
int tsfi_fips1_ascii_to_ebcdic(const char *ascii_in, uint8_t *ebcdic_out, int len);
int tsfi_fips1_ebcdic_to_ascii(const uint8_t *ebcdic_in, char *ascii_out, int len);

// Scenario 156: NBS FIPS PUB 16-1 Serial Bit Sequence & Parity Checker
int tsfi_fips16_serialize(uint8_t byte, int use_even_parity, uint16_t *out_bits);
int tsfi_fips16_deserialize(uint16_t bits, int use_even_parity, uint8_t *out_byte);

// Scenario 157: NBS FIPS PUB 41 Access Control Security Levels
int tsfi_fips41_authorize(const char *user_role, const char *action);

// Scenario 158: NBS FIPS PUB 113 Computer Data Authentication (DAA / DES-MAC)
int tsfi_fips113_generate_mac(tsfi_crypto_subsystem *crypto, const uint8_t *data, int len, uint8_t *out_mac, int supervisor_state);
int tsfi_fips113_verify_mac(tsfi_crypto_subsystem *crypto, const uint8_t *data, int len, const uint8_t *expected_mac, int supervisor_state);

// Scenario 159: NBS FIPS PUB 100 / X.25 Packet Network Interface
int tsfi_fips100_encapsulate(const uint8_t *payload, int len, uint16_t channel, uint8_t *out_packet, int *out_len);
int tsfi_fips100_decapsulate(const uint8_t *packet, int len, uint8_t *out_payload, int *out_len, uint16_t *out_channel);

// Scenario 160: NBS FIPS PUB 112 Password Security Validator
int tsfi_fips112_validate_password(const char *password, int *out_complexity_score);

// Scenario 161: NBS FIPS PUB 120 Graphical Kernel System (GKS) Primitive Parser
int tsfi_fips120_parse_gks_primitive(const uint8_t *stream, int len, int *out_primitive_type, int *out_point_count);

// Scenario 162: NBS FIPS PUB 95 Agency Code Validator
int tsfi_fips95_resolve_agency(const char *agency_code, char *out_name, int max_len);

// Scenario 163: NBS FIPS PUB 127 SQL Syntax Validator
int tsfi_fips127_validate_sql(const char *sql_query);

#endif // TSFI_MAINFRAME_FIPS_H
