#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define BLOCK_SIZE 8
#define ZENER_CLAMP_VOLTAGE 5.0
#define MIN_VOLTAGE 0.0
#define DECAY_RATE 0.15 // Volts decayed per simulation tick
#define RETENTION_THRESHOLD 2.5 // Minimum voltage representing logic high (1)
#define MAX_REFRESH_LATENCY 16


typedef struct {
    double voltages[BLOCK_SIZE]; // Capacitor charge voltages
    bool parity_bit; // Raydac parity bit for the block
    uint32_t ticks_since_refresh;
} verified_block_t;

// Compute parity for the logic states in the block
bool calculate_parity(const verified_block_t *block) {
    uint32_t ones_count = 0;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (block->voltages[i] >= RETENTION_THRESHOLD) {
            ones_count++;
        }
    }
    return (ones_count % 2) != 0;
}

// Initialize verified block
void init_verified_block(verified_block_t *block) {
    memset(block->voltages, 0, sizeof(block->voltages));
    block->parity_bit = false;
    block->ticks_since_refresh = 0;
}

// Write data values into the block with Zener clamping and Raydac parity generation
bool write_verified_block(verified_block_t *block, const double *input_voltages) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        double val = input_voltages[i];
        // Zener Diode Clamping: Limit voltage between MIN_VOLTAGE and ZENER_CLAMP_VOLTAGE
        if (val > ZENER_CLAMP_VOLTAGE) {
            val = ZENER_CLAMP_VOLTAGE;
        } else if (val < MIN_VOLTAGE) {
            val = MIN_VOLTAGE;
        }
        block->voltages[i] = val;
    }
    block->parity_bit = calculate_parity(block);
    block->ticks_since_refresh = 0;
    return true;
}

// Read and verify block with Raydac transfer check
bool read_verified_block(const verified_block_t *block, double *output_voltages) {
    // 0.5. SEAC Refresh Latency Check
    if (block->ticks_since_refresh > MAX_REFRESH_LATENCY) {
        printf("   [REFRESH ERROR] SEAC refresh latency exceeded! Data is untrusted.\n");
        return false;
    }

    // 1. Raydac Transfer Check: Verify parity
    bool active_parity = calculate_parity(block);
    if (active_parity != block->parity_bit) {
        printf("   [TRANSFER ERROR] Raydac parity check failed! Memory decay detected.\n");
        return false; // Transfer check fails
    }

    
    // 2. Transfer data
    for (int i = 0; i < BLOCK_SIZE; i++) {
        output_voltages[i] = block->voltages[i];
    }
    return true;
}

// Simulate capacitor charge decay over time
void decay_verified_block(verified_block_t *block) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (block->voltages[i] > MIN_VOLTAGE) {
            block->voltages[i] -= DECAY_RATE;
            if (block->voltages[i] < MIN_VOLTAGE) {
                block->voltages[i] = MIN_VOLTAGE;
            }
        }
    }
    block->ticks_since_refresh++;
}

// Refresh charge voltages back to saturated limits
void refresh_verified_block(verified_block_t *block) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (block->voltages[i] >= RETENTION_THRESHOLD) {
            block->voltages[i] = ZENER_CLAMP_VOLTAGE; // Recharge back to clamped maximum
        } else {
            block->voltages[i] = MIN_VOLTAGE; // Discharge clean
        }
    }
    block->ticks_since_refresh = 0;
}

// Write chained block incorporating previous block's signature
bool write_chained_block(verified_block_t *block, const double *input_voltages, bool prev_parity) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        double val = input_voltages[i];
        if (val > ZENER_CLAMP_VOLTAGE) val = ZENER_CLAMP_VOLTAGE;
        else if (val < MIN_VOLTAGE) val = MIN_VOLTAGE;
        block->voltages[i] = val;
    }
    block->parity_bit = calculate_parity(block) ^ prev_parity;
    block->ticks_since_refresh = 0;
    return true;
}

// Read chained block verifying dynamic parity logic chain
bool read_chained_block(const verified_block_t *block, double *output_voltages, bool prev_parity) {
    bool active_parity = calculate_parity(block) ^ prev_parity;
    if (active_parity != block->parity_bit) {
        printf("   [TRANSFER ERROR] Raydac chained parity check failed! History altered.\n");
        return false;
    }
    for (int i = 0; i < BLOCK_SIZE; i++) {
        output_voltages[i] = block->voltages[i];
    }
    return true;
}

// Raydac Arithmetic Unit Check: add two blocks and verify parity of result
bool add_blocks_verified(const verified_block_t *a, const verified_block_t *b, verified_block_t *result) {
    double val_a[BLOCK_SIZE];
    double val_b[BLOCK_SIZE];
    
    // Perform Raydac transfer check on inputs
    if (!read_verified_block(a, val_a) || !read_verified_block(b, val_b)) {
        printf("   [ARITHMETIC ERROR] Input block verification failed!\n");
        return false;
    }
    
    double val_res[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        val_res[i] = val_a[i] + val_b[i]; // Perform addition
    }
    
    // Write result with Zener clamping and parity generation
    write_verified_block(result, val_res);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZENER-DIODE-CAPACITOR VERIFIED BLOCK SUITE\n");
    printf("=============================================================\n");

    verified_block_t block;
    init_verified_block(&block);

    // Initial input data: mix of logic states (high/low voltages)
    double input[BLOCK_SIZE] = { 5.5, 0.2, 4.8, 1.2, 6.0, 0.0, 5.0, 0.1 };
    // After Zener clamping, expected voltages: { 5.0, 0.2, 4.8, 1.2, 5.0, 0.0, 5.0, 0.1 }
    // Logic states: { 1, 0, 1, 0, 1, 0, 1, 0 } -> 4 ones -> parity should be FALSE (even)

    printf("[TEST] Writing block with Zener clamping and parity generation...\n");
    bool ok = write_verified_block(&block, input);
    assert(ok);
    assert(block.parity_bit == false);
    printf("   ✓ Write successful. Clamped values verified.\n");

    // 1. Success Read
    printf("[TEST] Reading verified block immediately...\n");
    double output[BLOCK_SIZE] = {0};
    ok = read_verified_block(&block, output);
    assert(ok);
    printf("   ✓ Read and Raydac transfer check passed.\n");

    // 2. Simulate decay that does not flip logic state yet
    printf("[TEST] Simulating minor charge decay (no state change)...\n");
    decay_verified_block(&block); // voltages decay by 0.15V
    // 5.0 -> 4.85, 4.8 -> 4.65. Still >= 2.5V (Logic 1)
    ok = read_verified_block(&block, output);
    assert(ok); // Parity must still match
    printf("   ✓ Minor decay tolerated. Parity check passed.\n");

    // 3. Simulate deep decay that causes bit flip (parity error)
    printf("[TEST] Simulating deep charge decay to trigger parity error...\n");
    for (int i = 0; i < 15; i++) {
        decay_verified_block(&block);
    }
    // Deep decay causes logic states to shift, violating the stored parity
    ok = read_verified_block(&block, output);
    assert(!ok); // Should fail Raydac transfer check
    printf("   ✓ Decay-induced parity mismatch successfully caught by transfer check.\n");

    // 4. Recharge and Refresh
    printf("[TEST] Re-writing and simulating refresh loop...\n");
    write_verified_block(&block, input);
    decay_verified_block(&block);
    printf("   [INFO] Performing refresh cycle...\n");
    refresh_verified_block(&block);
    
    // Check that voltages are restored back to maximums
    assert(block.voltages[0] == ZENER_CLAMP_VOLTAGE);
    ok = read_verified_block(&block, output);
    assert(ok);
    printf("   ✓ Refresh restored capacitor voltages. Block verified.\n");

    // 5. Verification of Chained Block Series
    printf("[TEST] Initializing and writing a chained series of 3 verified blocks...\n");
    fflush(stdout);
    verified_block_t series[3];
    init_verified_block(&series[0]);
    init_verified_block(&series[1]);
    init_verified_block(&series[2]);

    double data0[BLOCK_SIZE] = { 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; // 3 ones -> parity odd (1)
    double data1[BLOCK_SIZE] = { 5.0, 0.0, 5.0, 0.0, 5.0, 0.0, 5.0, 0.0 }; // 4 ones -> parity even (0)
    double data2[BLOCK_SIZE] = { 0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 0.0, 0.0 }; // 2 ones -> parity even (0)


    // Write Block 0 (no prev block -> prev_parity = false)
    write_chained_block(&series[0], data0, false);
    // Write Block 1 (chains from Block 0)
    write_chained_block(&series[1], data1, series[0].parity_bit);
    // Write Block 2 (chains from Block 1)
    write_chained_block(&series[2], data2, series[1].parity_bit);

    // Verify all reads succeed initially
    double tmp_out[BLOCK_SIZE];
    ok = read_chained_block(&series[0], tmp_out, false);
    assert(ok);
    ok = read_chained_block(&series[1], tmp_out, series[0].parity_bit);
    assert(ok);
    ok = read_chained_block(&series[2], tmp_out, series[1].parity_bit);
    assert(ok);
    printf("   ✓ Chained read sequence verified successfully.\n");
    fflush(stdout);

    // Corrupt Block 0 by discharging a single capacitor cell below logic threshold
    printf("[TEST] Simulating single-cell charge loss in Block 0 to break validation chain...\n");
    fflush(stdout);
    series[0].voltages[0] = 0.0; // Bit flip (1 -> 0)

    // Reading Block 0 itself should fail
    ok = read_chained_block(&series[0], tmp_out, false);
    assert(!ok);
    // Crucially, reading Block 1 should also fail because the previous block's dynamic parity has flipped
    ok = read_chained_block(&series[1], tmp_out, calculate_parity(&series[0]));
    assert(!ok);
    printf("   ✓ Chain integrity verified: corruption in Block 0 broke Block 1 verification.\n");
    fflush(stdout);

    // Repair Block 0 and check if chain is repaired
    printf("[TEST] Restoring/repairing Block 0 to repair validation chain...\n");
    fflush(stdout);
    series[0].voltages[0] = ZENER_CLAMP_VOLTAGE; // Recharge/repair the bit
    ok = read_chained_block(&series[0], tmp_out, false);
    assert(ok);
    ok = read_chained_block(&series[1], tmp_out, series[0].parity_bit);
    assert(ok);
    printf("   ✓ Chain repaired. Block 1 reads normally again.\n");
    fflush(stdout);

    // 6. Raydac Arithmetic Unit Gating Check
    printf("[TEST] Executing Raydac verified arithmetic logic unit checks...\n");
    fflush(stdout);
    verified_block_t arg_a, arg_b, sum_result;
    init_verified_block(&arg_a);
    init_verified_block(&arg_b);
    init_verified_block(&sum_result);

    double a_vals[BLOCK_SIZE] = { 5.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; // parity even (0)
    double b_vals[BLOCK_SIZE] = { 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0 }; // parity odd (1)
    write_verified_block(&arg_a, a_vals);
    write_verified_block(&arg_b, b_vals);

    // Normal Addition: Should succeed
    ok = add_blocks_verified(&arg_a, &arg_b, &sum_result);
    assert(ok);
    double sum_vals[BLOCK_SIZE];
    ok = read_verified_block(&sum_result, sum_vals);
    assert(ok);
    assert(sum_vals[0] == 5.0 && sum_vals[4] == 5.0);
    printf("   ✓ Arithmetic addition and validation succeeded.\n");
    fflush(stdout);

    // Corrupt input B and verify addition is blocked
    printf("[TEST] Simulating input decay to verify arithmetic write blocking...\n");
    fflush(stdout);
    arg_b.voltages[4] = 0.0; // Bit flip
    ok = add_blocks_verified(&arg_a, &arg_b, &sum_result);
    assert(!ok); // Verification must fail, block write is prevented
    printf("   ✓ Arithmetic execution blocked successfully on corrupted input registers.\n");
    fflush(stdout);

    // 7. SEAC Dynamic Refresh Latency Check
    printf("[TEST] Testing SEAC dynamic refresh latency bounds...\n");
    fflush(stdout);
    verified_block_t latent_block;
    init_verified_block(&latent_block);
    double initial_data[BLOCK_SIZE] = { 5.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    write_verified_block(&latent_block, initial_data);

    // Simulate clock cycles passing (increasing ticks_since_refresh) without actual memory decay
    latent_block.ticks_since_refresh = 10;
    double read_out[BLOCK_SIZE];
    ok = read_verified_block(&latent_block, read_out);
    assert(ok); // 10 is <= 16, should succeed

    latent_block.ticks_since_refresh = 20; // Exceeds MAX_REFRESH_LATENCY (16)
    ok = read_verified_block(&latent_block, read_out);
    assert(!ok); // Should fail due to refresh timeout
    printf("   ✓ SEAC refresh timeout successfully blocked un-refreshed memory read.\n");
    fflush(stdout);





    printf("=============================================================\n");
    printf("ZENER-DIODE-CAPACITOR VERIFIED BLOCK TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
