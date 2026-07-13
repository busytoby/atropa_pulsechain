#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

// Universal Opcodes
typedef enum {
    YUL_OP_GAS_TRANSFER     = 0x20,
    YUL_OP_QUERY_READ       = 0x30
} YulOpcode;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    PPN      ppn;
    uint64_t recipient;
    uint64_t value;
    YulOpcode d_channel;
} TwoChannelTx;

// Global state variables simulating database
uint64_t g_balances[100]; 
const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

uint64_t get_balance(PPN ppn) {
    return g_balances[get_ppn_slot(ppn)];
}

void set_balance(PPN ppn, uint64_t balance) {
    g_balances[get_ppn_slot(ppn)] = balance;
}

// Coaxial execution pipeline
inline bool process_coaxial_transaction_bench(TwoChannelTx *tx) {
    PPN caller = tx->ppn;
    uint32_t slot = get_ppn_slot(caller);
    uint64_t balance = g_balances[slot];
    
    // Check Gas
    if (balance < UNIVERSAL_GAS_FEE) {
        return false;
    }
    
    // Deduct gas
    g_balances[slot] = balance - UNIVERSAL_GAS_FEE;
    
    if (tx->d_channel == YUL_OP_QUERY_READ) {
        // Read Operation
        volatile uint64_t val = g_balances[slot];
        (void)val;
    } else if (tx->d_channel == YUL_OP_GAS_TRANSFER) {
        // Write/Transfer Operation
        uint64_t usable_bal = balance - UNIVERSAL_GAS_FEE;
        if (usable_bal < tx->value) {
            return false;
        }
        uint32_t rec_slot = tx->recipient % 100;
        g_balances[slot] = usable_bal - tx->value;
        g_balances[rec_slot] += tx->value;
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: COAXIAL BUS TRANS-PER-SECOND BENCHMARK\n");
    printf("=============================================================\n");

    // Initialize balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN caller = {10, 5};
    set_balance(caller, 100000000); // 100 million gas tokens

    // Prepare transaction payload
    TwoChannelTx tx = {
        .ppn = caller,
        .recipient = 6,
        .value = 10,
        .d_channel = YUL_OP_GAS_TRANSFER
    };

    const uint32_t iterations = 1000000; // 1 million transactions
    printf("Running %u coaxial transactions...\n", iterations);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (uint32_t i = 0; i < iterations; i++) {
        bool res = process_coaxial_transaction_bench(&tx);
        if (!res) {
            printf("Error at iteration %u\n", i);
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double tps = iterations / elapsed;
    double latency = (elapsed / iterations) * 1e9; // in nanoseconds

    printf("\nBenchmark Results:\n");
    printf("-------------------------------------------------------------\n");
    printf("Total Elapsed Time   : %.4f seconds\n", elapsed);
    printf("Transactions / Sec   : %.2f TPS\n", tps);
    printf("Average Latency      : %.2f ns per transaction\n", latency);
    printf("-------------------------------------------------------------\n");
    
    // Assert latency is under the sub-microsecond threshold (< 1000 ns)
    assert(latency < 1000.0);
    printf("PASS: Latency satisfies the sub-microsecond performance guard gate!\n");
    printf("=============================================================\n");
    return 0;
}
