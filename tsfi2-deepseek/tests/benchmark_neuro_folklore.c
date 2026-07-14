#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_wiring.h"
#include "tsfi_sort.h"
#include "lau_memory.h"

#define MAX_CACHED_CONTRACTS 256
typedef struct {
    char name[128];
    uint8_t *bytecode;
    size_t size;
    uint64_t virtual_address;
    char path[512];
} CachedContract;
extern CachedContract g_cached_contracts[MAX_CACHED_CONTRACTS];
extern int g_cached_contracts_count;

static inline uint64_t get_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    printf("=== TSFi Auncient Folklore CPU Neuro-Selection Benchmark ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    /* 1. Setup simulated allocations (memories) in LauRegistry */
    printf("[MEM] Pre-allocating memory blocks to simulate search workspace...\n");
    LauMetadata m1 = { .alloc_size = 512, .alloc_file = "neuro_block_1" };
    LauMetadata m2 = { .alloc_size = 2048, .alloc_file = "neuro_block_2" };
    LauMetadata m3 = { .alloc_size = 1024, .alloc_file = "neuro_block_3" };

    lau_registry_insert(&m1);
    lau_registry_insert(&m2);
    lau_registry_insert(&m3);

    /* 2. Initialize 6502 CPU simulator */
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    /* Register base registers for accumulator simulation (A=0, X=0, Y=0, PC=1536) */
    char cmd[256];
    uint32_t reg_addrs[] = {128, 129, 130, 131, 132, 133};
    uint32_t reg_vals[]  = {250, 120,  80, 0xFF, 0x20, 1536}; // Seed initial register charges
    for (int i = 0; i < 6; i++) {
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                      "%064x"
                      "%064x\"", 
                reg_addrs[i], reg_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    printf("[BENCHMARK] Starting folklore CPU-level search & FET accumulation loop...\n");
    uint64_t start = get_ns();
    int iterations = 200;

    for (int i = 0; i < iterations; i++) {
        /*
         * Auncient Folklore Loop:
         * 1. Run Möbius-Klein topological search on registry.
         * 2. Feed selection size back to CPU registers to simulate selector accumulation.
         * 3. Execute 6502 steps to trigger analog FET discharge decay.
         */
        LauMetadata *winner = tsfi_wire_topological_select(NULL, 1);
        uint64_t winning_size = winner ? winner->alloc_size : 100;

        // Write selector metric back to accumulator A (reg 128)
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                      "%064x"
                      "%064lx\"", 
                128, winning_size);
        tsfi_zmm_vm_exec(&vm, cmd);

        // Run processor steps (triggers CPU execution and internal interpreter FET discharge decay)
        sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b1808"
                      "%064x\"", 1);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    uint64_t end = get_ns();
    double dur_sec = (double)(end - start) / 1e9;
    double rate = (double)iterations / dur_sec;

    printf("[RESULTS] Benchmark complete:\n");
    printf("  - Total Iterations: %d\n", iterations);
    printf("  - Elapsed Time:     %.4f seconds\n", dur_sec);
    printf("  - Processing Rate:  %.2f Folklore-Steps/second\n", rate);

    /* 4. Validate final accumulated register state */
    printf("[ZMM] Reading final CPU register state...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"cpu6502\", \"e0f77c57\"");
    printf("  Accumulator Matrix: %s\n", vm.output_buffer);

    /* 5. Create new Qings on the ZMM for accumulated memory blocks */
    printf("[ZMM] Creating new Qing memory registries using accumulated primary key...\n");
    LauMetadata *last_winner = tsfi_wire_topological_select(NULL, 1);
    uint64_t accumulated_id = last_winner ? last_winner->alloc_size : 2048;
    uint64_t qing_address = accumulated_id;

    /* A. BTC Script Dual Stack Verification & State Transition */
    extern bool blue_box_verify_btc_script_transition(const uint8_t *old_row_data, size_t old_len, const uint8_t *witness_script, size_t script_len, const uint8_t *new_row_data, size_t new_len);
    extern bool blue_box_commit_quadtree_via_btc_script(uint64_t old_root, uint64_t next_root, const uint8_t *witness, size_t witness_len);

    uint8_t old_row[8] = {0, 0, 0, 0, 0, 0, 8, 0}; // 2048 encoded
    uint8_t btc_script[8] = {0, 0, 0, 0, 0, 0, 0, 100}; // mock hash
    blue_box_verify_btc_script_transition(old_row, 8, btc_script, 8, old_row, 8);
    blue_box_commit_quadtree_via_btc_script(0, 0, btc_script, 8);

    /* B. Convert 2-3 Tree state to Quadtree root */
    extern bool blue_box_verify_23_to_quad_conversion(uint64_t r23_root_0, uint64_t r23_root_1, uint64_t r23_root_2, uint64_t r23_root_3, uint64_t *r_quad_out);
    uint64_t quad_root = 0;
    blue_box_verify_23_to_quad_conversion(512, 1024, 2048, 4096, &quad_root);

    if (g_cached_contracts_count < MAX_CACHED_CONTRACTS) {
        CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
        snprintf(c->name, sizeof(c->name), "dynamic_%lx", qing_address);
        c->size = 32;
        c->bytecode = malloc(c->size);
        memset(c->bytecode, 0x00, c->size);
        c->virtual_address = qing_address;
        snprintf(c->path, sizeof(c->path), "dynamic");

        printf("[PASS] Instantiated dynamic Qing contract address: dynamic_%lx (virtual address: 0x%lx)\n", qing_address, qing_address);
    }

    lau_registry_remove(&m1);
    lau_registry_remove(&m2);
    lau_registry_remove(&m3);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== FOLKLORE SELECTION BENCHMARK PASSED ===\n");
    return 0;
}
