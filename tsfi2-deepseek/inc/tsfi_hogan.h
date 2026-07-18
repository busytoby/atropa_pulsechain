#ifndef TSFI_HOGAN_H
#define TSFI_HOGAN_H

#include <stdint.h>
#include <stddef.h>

#define HOGAN_MAX_TRANSACTIONS 1024
#define HOGAN_MAX_ACCOUNTS 128

typedef enum {
    VM_EVM,
    VM_ZMM,
    VM_RAMAC
} hogan_vm_type;

typedef struct {
    uint32_t tx_id;
    uint32_t sender_id;
    uint32_t recipient_id;
    uint64_t amount;
    hogan_vm_type target_vm;
    uint8_t processed;
} hogan_transaction;

typedef struct {
    uint32_t account_id;
    uint64_t balance;
    uint8_t active;
} hogan_account;

typedef struct {
    hogan_account accounts[HOGAN_MAX_ACCOUNTS];
    hogan_transaction tx_log[HOGAN_MAX_TRANSACTIONS];
    size_t tx_count;
    uint8_t live_processing_enabled;
    uint32_t current_epoch;
    uint8_t acab_epoch_root[32];
} hogan_umbrella_system;

// 1. Unified Umbrella Architecture
void tsfi_hogan_init(hogan_umbrella_system *sys);
int tsfi_hogan_register_account(hogan_umbrella_system *sys, uint32_t account_id, uint64_t initial_balance);
int tsfi_hogan_dispatch_tx(hogan_umbrella_system *sys, uint32_t sender_id, uint32_t recipient_id, uint64_t amount, hogan_vm_type target_vm);

// 2. Logical File System (LFS) Virtualization
int tsfi_hogan_lfs_save(const hogan_umbrella_system *sys, const char *filepath);
int tsfi_hogan_lfs_load(hogan_umbrella_system *sys, const char *filepath);

// 3. Overnight Batch Reconciliation (Epoch Sync)
int tsfi_hogan_overnight_reconciliation(hogan_umbrella_system *sys, const char *lfs_filepath);

// Sequential Block-Record Queue (BLACK Box)
#define HOGAN_MAX_FIELDS 16

typedef enum {
    FIELD_TYPE_UINT32,
    FIELD_TYPE_UINT64,
    FIELD_TYPE_STRING
} hogan_field_type;

typedef struct {
    char name[16];
    uint32_t offset;
    hogan_field_type type;
} hogan_field_def;

typedef struct {
    hogan_field_def fields[HOGAN_MAX_FIELDS];
    size_t field_count;
} hogan_record_dict;

int tsfi_hogan_write_seq_record(const char *filepath, const uint8_t *payload, size_t size);
int tsfi_hogan_read_seq_record(const char *filepath, size_t index, uint8_t *payload_out, size_t *size_out);

// Runtime Record Dictionary Resolver (RED Box)
void tsfi_hogan_init_dict(hogan_record_dict *dict);
int tsfi_hogan_add_field(hogan_record_dict *dict, const char *name, uint32_t offset, hogan_field_type type);
int tsfi_hogan_resolve_uint32(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint32_t *val_out);
int tsfi_hogan_resolve_uint64(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint64_t *val_out);

// Sequential Journal Recovery (Transaction Playback)
int tsfi_hogan_write_journal(const char *filepath, const hogan_transaction *tx);
int tsfi_hogan_replay_journal(hogan_umbrella_system *sys, const char *filepath);

// Sequential Activity Audit Log (System Auditing)
typedef struct {
    uint32_t epoch;
    uint32_t account_id;
    char activity_type[16];
    uint8_t status;
} hogan_audit_entry;

int tsfi_hogan_write_audit_log(const char *filepath, uint32_t epoch, uint32_t account_id, const char *activity_type, uint8_t status);
int tsfi_hogan_print_audit_trail(const char *filepath, size_t *entries_count_out);

// Master Account Index Resolver (Storage Record Pointer Addressing)
typedef struct {
    uint32_t account_id;
    uint32_t sector_offset;
} hogan_index_entry;

int tsfi_hogan_write_account_index(const char *filepath, uint32_t account_id, uint32_t sector_offset);
int tsfi_hogan_lookup_account_offset(const char *filepath, uint32_t account_id, uint32_t *sector_offset_out);

#endif // TSFI_HOGAN_H
