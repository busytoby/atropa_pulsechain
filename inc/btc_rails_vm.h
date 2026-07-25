#ifndef BTC_RAILS_VM_H
#define BTC_RAILS_VM_H

#include <stdint.h>
#include <stddef.h>

/* MOS 6526 CIA Timer Emulation Registers */
typedef struct {
    uint16_t timer_a_latch;
    uint16_t timer_a_count;
    uint16_t timer_b_latch;
    uint16_t timer_b_count;
    uint8_t control_a;
    uint8_t control_b;
    uint8_t port_a_data;
    uint8_t port_b_data;
    uint8_t sdr; /* Serial Data Register */
} M6526Cia;

/* 2-3 Tree Node Structure for Transaction Indexing */
typedef struct TwoThreeNode {
    int is_leaf;
    int key_count;
    uint32_t keys[2];
    uint8_t *data_payloads[2];
    size_t payload_sizes[2];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

/* VM Engine Configuration using Dual-CIA Stack Control */
typedef struct {
    M6526Cia cia1; /* Governs the Data Stack (DS) */
    M6526Cia cia2; /* Governs the Return Stack (RS) */
    uint8_t *sram_pool;
    size_t sram_size;
    TwoThreeNode *root_index;
} BtcRailsVm;

/* Initialization and Lifecycle Operations */
BtcRailsVm *btc_rails_vm_init(size_t sram_size);
void btc_rails_vm_free(BtcRailsVm *vm);

/* Stack Push and Pop Operations clocked via CIA Timers */
int btc_rails_vm_push_ds(BtcRailsVm *vm, const uint8_t *data, size_t size);
int btc_rails_vm_pop_ds(BtcRailsVm *vm, uint8_t *data_out, size_t max_size);
int btc_rails_vm_push_rs(BtcRailsVm *vm, const uint8_t *data, size_t size);
int btc_rails_vm_pop_rs(BtcRailsVm *vm, uint8_t *data_out, size_t max_size);

/* 2-3 Tree operations (Registry Indexing) */
TwoThreeNode *two_three_tree_insert(TwoThreeNode *root, uint32_t key, const uint8_t *payload, size_t size);
uint8_t *two_three_tree_search(TwoThreeNode *root, uint32_t key, size_t *size_out);
void two_three_tree_free(TwoThreeNode *node);

/* .dat.bin Database File Loading */
int btc_rails_vm_load_dat(BtcRailsVm *vm, const char *dat_bin_path);

/* Yul Deployment and Execution */
int btc_rails_vm_deploy_yul(BtcRailsVm *vm, const uint8_t *bytecode, size_t size);

#endif /* BTC_RAILS_VM_H */
