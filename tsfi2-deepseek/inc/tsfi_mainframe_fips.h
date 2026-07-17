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

#endif // TSFI_MAINFRAME_FIPS_H
