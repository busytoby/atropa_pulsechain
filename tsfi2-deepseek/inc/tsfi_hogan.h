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

#define STATUS_ACTIVE     0
#define STATUS_STOP_ALL   1
#define STATUS_STOP_DEBIT 2

typedef struct {
    uint32_t account_id;
    uint64_t balance;
    uint8_t active;
    uint32_t backup_account_id;
    uint8_t has_backup;
    uint64_t balance_held;
    uint8_t status_code;
    uint64_t daily_limit;
    uint64_t daily_spent;
    uint64_t daily_transfer_limit;
    uint64_t daily_transferred;
    uint8_t is_frozen;
    uint8_t fee_exempt;
    uint32_t custom_interest_rate_bps;
    uint8_t has_custom_rate;
    uint64_t min_balance;
    uint32_t daily_tx_limit;
    uint32_t daily_tx_count;
    uint8_t priority_tier;
    uint64_t max_balance;
} hogan_account;

#define HOGAN_MAX_BLOCKED_CARDS 32
#define CARD_STATUS_ACTIVE      0
#define CARD_STATUS_BLOCKED     1

typedef struct {
    hogan_account accounts[HOGAN_MAX_ACCOUNTS];
    hogan_transaction tx_log[HOGAN_MAX_TRANSACTIONS];
    size_t tx_count;
    uint8_t live_processing_enabled;
    uint32_t current_epoch;
    uint8_t acab_epoch_root[32];
    uint32_t blocked_cards[HOGAN_MAX_BLOCKED_CARDS];
    size_t blocked_card_count;
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

// Account Ledger Statement Generator (Statement Reporting)
typedef struct {
    uint32_t account_id;
    uint64_t amount;
    uint8_t is_credit;
    char description[24];
} hogan_ledger_entry;

int tsfi_hogan_write_ledger_entry(const char *filepath, uint32_t account_id, uint64_t amount, uint8_t is_credit, const char *description);
int tsfi_hogan_print_statement(const char *filepath, uint32_t account_id, size_t *entries_count_out);

// Batch Interest Posting Engine (Ledger Calculation Updates)
typedef struct {
    uint32_t account_id;
    uint64_t original_balance;
    uint64_t interest_added;
    uint64_t new_balance;
} hogan_interest_entry;

int tsfi_hogan_apply_interest(hogan_umbrella_system *sys, const char *filepath, uint32_t rate_bps);

// Batch Fee Application Engine (Ledger Maintenance Deductions)
typedef struct {
    uint32_t account_id;
    uint64_t original_balance;
    uint64_t fee_deducted;
    uint64_t new_balance;
} hogan_fee_entry;

int tsfi_hogan_apply_fees(hogan_umbrella_system *sys, const char *filepath, uint64_t flat_fee);

// Overdraft Protection Engine (Backup Balance Linkage)
typedef struct {
    uint32_t primary_account_id;
    uint32_t backup_account_id;
    uint64_t amount_transferred;
    uint8_t success;
} hogan_overdraft_entry;

int tsfi_hogan_link_backup(hogan_umbrella_system *sys, uint32_t primary_id, uint32_t backup_id);
int tsfi_hogan_overnight_reconciliation_ex(hogan_umbrella_system *sys, const char *lfs_filepath, const char *overdraft_filepath);

// Card Authorization Engine (Temporary Balance Holds)
typedef struct {
    uint32_t card_id;
    uint32_t account_id;
    uint32_t merchant_id;
    uint64_t amount;
    uint8_t approved;
} hogan_card_entry;

int tsfi_hogan_authorize_card(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint32_t account_id, uint32_t merchant_id, uint64_t amount);

// Administrative Account Stop Manager (Status Overrides)
typedef struct {
    uint32_t account_id;
    uint8_t previous_status;
    uint8_t new_status;
    uint32_t authority_id;
} hogan_stop_entry;

int tsfi_hogan_apply_account_stop(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t new_status, uint32_t authority_id);

// Card Hold Release Engine (Balance Hold Settlements)
typedef struct {
    uint32_t card_id;
    uint32_t account_id;
    uint64_t amount_released;
    uint8_t success;
} hogan_release_entry;

int tsfi_hogan_release_hold(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint32_t account_id, uint64_t amount_released);

// Daily Spending Limit Manager (Credit Limit Overrides)
typedef struct {
    uint32_t account_id;
    uint64_t previous_limit;
    uint64_t new_limit;
    uint32_t authority_id;
} hogan_limit_entry;

int tsfi_hogan_update_daily_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id);

// Account Closure and Liquidation Engine (Status Termination)
typedef struct {
    uint32_t account_id;
    uint64_t liquidated_balance;
    uint32_t authority_id;
} hogan_closure_entry;

int tsfi_hogan_close_account(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t authority_id);

// Daily Transfer Limit Manager (Outbound Transfer Limits)
typedef struct {
    uint32_t account_id;
    uint64_t previous_limit;
    uint64_t new_limit;
    uint32_t authority_id;
} hogan_trans_limit_entry;

int tsfi_hogan_update_transfer_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id);

// Standing Order Scheduler (Scheduled Recurring Transfers)
typedef struct {
    uint32_t sender_id;
    uint32_t recipient_id;
    uint64_t amount;
} hogan_standing_order;

int tsfi_hogan_register_standing_order(const char *filepath, uint32_t sender_id, uint32_t recipient_id, uint64_t amount);
int tsfi_hogan_execute_standing_orders(hogan_umbrella_system *sys, const char *filepath);

// Account Compliance Freeze Manager (Legal Holds)
typedef struct {
    uint32_t account_id;
    uint8_t is_frozen;
    uint32_t authority_id;
} hogan_freeze_entry;

int tsfi_hogan_apply_account_freeze(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t is_frozen, uint32_t authority_id);

// Card Status Compliance Manager (Card Locks)
typedef struct {
    uint32_t card_id;
    uint8_t previous_status;
    uint8_t new_status;
    uint32_t authority_id;
} hogan_card_status_entry;

int tsfi_hogan_apply_card_status(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint8_t new_status, uint32_t authority_id);

// Batch Fee Exemption Manager (Fee Waivers)
typedef struct {
    uint32_t account_id;
    uint8_t fee_exempt;
    uint32_t authority_id;
} hogan_exemption_entry;

int tsfi_hogan_apply_fee_exemption(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t fee_exempt, uint32_t authority_id);

// Interest Rate Override Manager (Custom Rates)
typedef struct {
    uint32_t account_id;
    uint32_t previous_rate_bps;
    uint32_t new_rate_bps;
    uint32_t authority_id;
} hogan_int_override_entry;

int tsfi_hogan_apply_interest_override(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_rate_bps, uint32_t authority_id);

// Minimum Balance Compliance Manager (Threshold Enforcement)
typedef struct {
    uint32_t account_id;
    uint64_t previous_min_balance;
    uint64_t new_min_balance;
    uint32_t authority_id;
} hogan_min_balance_entry;

int tsfi_hogan_update_min_balance(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_min_balance, uint32_t authority_id);

// Daily Transaction Count Manager (Velocity Capping)
typedef struct {
    uint32_t account_id;
    uint32_t previous_tx_limit;
    uint32_t new_tx_limit;
    uint32_t authority_id;
} hogan_tx_count_entry;

int tsfi_hogan_update_tx_count_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_limit, uint32_t authority_id);

// Account Priority Routing Manager (Priority Queuing)
typedef struct {
    uint32_t account_id;
    uint8_t previous_priority;
    uint8_t new_priority;
    uint32_t authority_id;
} hogan_priority_entry;

int tsfi_hogan_update_priority_tier(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t new_priority, uint32_t authority_id);

// Maximum Balance Compliance Manager (Deposit Caps)
typedef struct {
    uint32_t account_id;
    uint64_t previous_max_balance;
    uint64_t new_max_balance;
    uint32_t authority_id;
} hogan_max_balance_entry;

int tsfi_hogan_update_max_balance(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_max_balance, uint32_t authority_id);

#endif // TSFI_HOGAN_H
