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

#endif // TSFI_HOGAN_H
