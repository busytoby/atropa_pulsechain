#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi Helmholtz Helmholtz Establishment Test ===\n");
    tsfi_helmholtz_init();
    uint64_t world_chain_id = 2026;

    // 1. Allocate 1 MiB Manifold (512KB State + 512KB Lore)
    void *manifold = lau_malloc(1024 * 1024);
    memset(manifold, 0, 1024 * 1024);
    AR_AccountLeaf *leaves = (AR_AccountLeaf *)manifold;

    // 2. Setup AR Ledger (Leaf 512)
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Initializing AR Ledger Account 0 (Leaf 512)...\n");
    leaves[512].nonce = 1;
    // Set Owner Address (Ethereum format placeholder)
    memset(leaves[512].owner_address, 0xAA, 20);
    // Set Balance to 1000 AR
    memset(leaves[512].balance, 0, 32);
    leaves[512].balance[0] = 0xE8; // 1000 in little-endian (0x03E8)
    leaves[512].balance[1] = 0x03;

    // 3. Establish Helmholtz Root for Epoch 1
    uint8_t root_epoch1[32];
    uint8_t receipt_epoch1[32];
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Establishing Helmholtz Roots for Epoch 1...\n");
    tsfi_helmholtz_reduce_11(root_epoch1, receipt_epoch1, NULL, NULL, manifold, 1, world_chain_id, NULL);
    
    printf("Epoch 1 State Root:   ");
    for(int i=0; i<32; i++) printf("%02x", root_epoch1[i]);
    printf("\n");
    printf("Epoch 1 Receipt Root: ");
    for(int i=0; i<32; i++) printf("%02x", receipt_epoch1[i]);
    printf("\n");

    // 4. Setup Directive List (The Wave Source)
    TSFiDirective *directives = calloc(TSFI_DIRECTIVE_LEAVES, sizeof(TSFiDirective));
    
    // Directive 0: Correct Nonce & ChainID & Rigid Signature (Success)
    directives[0].src_leaf = 512;
    directives[0].dst_leaf = 513;
    directives[0].nonce = 1; 
    directives[0].chain_id = world_chain_id;
    directives[0].amount[0] = 100;
    directives[0].gas_limit = 21000;
    
    // Construct rigid signature for Directive 0
    uint8_t d0_hash[32];
    tsfi_internal_zmm_hash(d0_hash, (const uint8_t*)&directives[0], 140);
    // Signature XOR-sum must match d0_hash[0]
    memset(directives[0].signature, 0, 64);
    directives[0].signature[0] = d0_hash[0];

    // Directive 3: Bad Signature (Authorization Failure)
    TSFiDirective bad_sig;
    memset(&bad_sig, 0, sizeof(bad_sig));
    bad_sig.src_leaf = 512;
    bad_sig.dst_leaf = 513;
    bad_sig.nonce = 2; // Should be 2 after first transfer
    bad_sig.chain_id = world_chain_id;
    bad_sig.amount[0] = 10;
    bad_sig.gas_limit = 21000;
    
    uint8_t d3_hash[32];
    tsfi_internal_zmm_hash(d3_hash, (const uint8_t*)&bad_sig, 140);
    // Construct an invalid signature: XOR-sum must NOT match d3_hash[0]
    memset(bad_sig.signature, 0, 64);
    bad_sig.signature[0] = d3_hash[0] ^ 0xFF; // Flip all bits to ensure mismatch

    // Directive 1: Bad Nonce (Boltzmann Phase Mismatch - Failure)
    directives[1].src_leaf = 600; 
    directives[1].dst_leaf = 513;
    directives[1].nonce = 999; 
    directives[1].chain_id = world_chain_id;
    directives[1].amount[0] = 50;
    directives[1].gas_limit = 21000;

    // Directive 2: Bad ChainID (Wave Frequency Mismatch - Failure)
    TSFiDirective bad_wave;
    memset(&bad_wave, 0, sizeof(bad_wave));
    bad_wave.src_leaf = 512;
    bad_wave.dst_leaf = 513;
    bad_wave.nonce = 2; // Nonce incremented by Directive 0
    bad_wave.chain_id = 9999; // WRONG WORLD
    bad_wave.amount[0] = 10;
    bad_wave.gas_limit = 21000;

    uint8_t directive_root[32];
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Establishing Directive Root (Wave Source) for Epoch 1...\n");
    tsfi_helmholtz_reduce_directives(directive_root, directives, 1, world_chain_id);
    
    printf("Directive Root:       ");
    for(int i=0; i<32; i++) printf("%02x", directive_root[i]);
    printf("\n");

    // 5. Modify State via JIT Thunks
    uint8_t access_map[64] = {0};
    _Atomic uint64_t energy_used = 0;
    ThunkProxy *proxy = ThunkProxy_create();
    uint64_t resonance_k = 1; // Start with fundamental frequency (everything matches)
    
    // Execute Success Directive (TX Index 0)
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Propagating Valid Wave (Correct Nonce/ChainID/Signature)...\n");
    void (*transfer_thunk)(void) = (void(*)(void))tsfi_helmholtz_emit_ar_transfer(proxy, manifold, access_map, &directives[0], &energy_used, world_chain_id, 0, resonance_k);
    ThunkProxy_seal(proxy);
    transfer_thunk();

    // Execute Failure Directive (Bad Nonce) (TX Index 1)
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Propagating Out-of-Phase Wave (Bad Nonce)...\n");
    ThunkProxy_unseal(proxy);
    void (*fail_thunk)(void) = (void(*)(void))tsfi_helmholtz_emit_ar_transfer(proxy, manifold, access_map, &directives[1], &energy_used, world_chain_id, 1, resonance_k);
    ThunkProxy_seal(proxy);
    fail_thunk();

    // Execute Failure Directive (Bad ChainID) (TX Index 2)
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Propagating Cross-World Wave (Bad ChainID)...\n");
    ThunkProxy_unseal(proxy);
    void (*bad_world_thunk)(void) = (void(*)(void))tsfi_helmholtz_emit_ar_transfer(proxy, manifold, access_map, &bad_wave, &energy_used, world_chain_id, 2, resonance_k);
    ThunkProxy_seal(proxy);
    bad_world_thunk();

    // Execute Failure Directive (Bad Signature) (TX Index 3)
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Propagating Unauthorized Wave (Bad Signature)...\n");
    ThunkProxy_unseal(proxy);
    void (*bad_sig_thunk)(void) = (void(*)(void))tsfi_helmholtz_emit_ar_transfer(proxy, manifold, access_map, &bad_sig, &energy_used, world_chain_id, 3, resonance_k);
    ThunkProxy_seal(proxy);
    bad_sig_thunk();

    // Test High-Impedance Resonance (TX Index 4)
    resonance_k = 0xFFFFFFFFFFFFFFFFULL; // High Frequency: likely mismatch
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Propagating High-Frequency Wave (Resonance Gating)...\n");
    TSFiDirective resonance_tx = directives[0];
    resonance_tx.nonce = 2; // Should be 2 now
    ThunkProxy_unseal(proxy);
    void (*resonance_thunk)(void) = (void(*)(void))tsfi_helmholtz_emit_ar_transfer(proxy, manifold, access_map, &resonance_tx, &energy_used, world_chain_id, 4, resonance_k);
    ThunkProxy_seal(proxy);
    resonance_thunk();

    // Verify Energy: 21000 * 5 = 105000 units expected
    if (energy_used == 105000) {
        printf("PASS: Helmholtz Energy correctly accounted for all 5 wave pulses (105000 units).\n");
    } else {
        printf("FAIL: Energy mismatch (%lu != 105000).\n", (uint64_t)energy_used);
    }

    // Verify Receipts
    TSFiReceipt *receipts = (TSFiReceipt *)((AR_AccountLeaf*)manifold + TSFI_STATE_LEAVES);
    if (receipts[0].status == 1 && receipts[0].energy_used == 21000) {
        printf("PASS: Valid signed wave established correctly (Receipt 0).\n");
    } else {
        printf("FAIL: Valid signed wave status mismatch (Receipt 0: Status=%lu, Energy=%lu).\n", receipts[0].status, receipts[0].energy_used);
    }
    
    if (receipts[3].status == 0 && receipts[3].energy_used == 84000) {
        printf("PASS: Unauthorized wave correctly rejected (Receipt 3).\n");
    } else {
        printf("FAIL: Unauthorized wave status mismatch (Receipt 3: Status=%lu, Energy=%lu).\n", receipts[3].status, receipts[3].energy_used);
    }
    
    if (receipts[1].status == 0 && receipts[1].energy_used == 42000) {
        printf("PASS: Out-of-phase wave correctly penalized (Receipt 1).\n");
    } else {
        printf("FAIL: Receipt 1 mismatch (Status=%lu, Energy=%lu).\n", receipts[1].status, receipts[1].energy_used);
    }

    if (receipts[2].status == 0 && receipts[2].energy_used == 63000) {
        printf("PASS: Cross-world wave correctly penalized (Receipt 2).\n");
    } else {
        printf("FAIL: Receipt 2 mismatch (Status=%lu, Energy=%lu).\n", receipts[2].status, receipts[2].energy_used);
    }

    // Verify Access Map: Leaves 512 and 513 map to bit (512/4) = 128
    // 128 / 8 = 16. So byte 16, bit 0 should be set.
    // Wait, the calculation in thunk is bit_idx = leaf_idx / 4;
    // leaf 512 / 4 = 128. 128 / 64 = 2 (qword 2). 128 % 64 = 0 (bit 0 of qword 2).
    // Qword 2 starts at byte 16.
    if (access_map[16] & 0x01) {
        printf("PASS: Access Map correctly marked region for Leaf 512/513.\n");
    } else {
        printf("FAIL: Access Map bit not set (Byte 16 = %02x).\n", access_map[16]);
    }

    // 5. Establish Helmholtz Root for Epoch 2
    uint8_t root_epoch2[32];
    uint8_t receipt_epoch2[32];
    TSFI_DEBUG_FPRINTF(stdout, "[TEST] Establishing Helmholtz Roots for Epoch 2...\n");
    tsfi_helmholtz_reduce_11(root_epoch2, receipt_epoch2, NULL, NULL, manifold, 2, world_chain_id, NULL);

    printf("Epoch 2 State Root:   ");
    for(int i=0; i<32; i++) printf("%02x", root_epoch2[i]);
    printf("\n");
    printf("Epoch 2 Receipt Root: ");
    for(int i=0; i<32; i++) printf("%02x", receipt_epoch2[i]);
    printf("\n");

    // 6. Verify Determinism
    assert(memcmp(root_epoch1, root_epoch2, 32) != 0);
    printf("PASS: Helmholtz Roots are unique and reflect state/temporal change.\n");

    ThunkProxy_destroy(proxy);
    free(directives);
    lau_free(manifold);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
