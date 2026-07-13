#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_ACCUMULATOR_LIMIT 1000
#define BASE_SLOT_DURATION    50

typedef struct {
    uint32_t active_slot;
    uint32_t slot_duration_ms;
    int64_t clock_drift_ns;
} TdmaState;

typedef struct {
    uint64_t accumulated_value;
    uint64_t rules_limit;
    bool rule_violation_flag;
} AccumulatorTable;

// Runs a clock cycle processing TDMA state and applying rules-based accumulation
void accumulate_yul_cycle(TdmaState *tdma, AccumulatorTable *table, uint64_t transaction_weight) {
    // 1. Accumulator Value Scaling: Scale transaction weight by slot duration factor
    uint64_t scaled_weight = transaction_weight * (tdma->slot_duration_ms / 10);

    // 2. Drift-Compensated Accumulation: Adjust weight based on clock drift
    if (tdma->clock_drift_ns > 0) {
        scaled_weight += (uint64_t)tdma->clock_drift_ns;
    } else if (tdma->clock_drift_ns < 0) {
        uint64_t magnitude = (uint64_t)(-tdma->clock_drift_ns);
        scaled_weight = (scaled_weight > magnitude) ? (scaled_weight - magnitude) : 0;
    }

    // 3. Deterministic Rules Validation: Ensure accumulated total does not cross limits
    if (table->accumulated_value + scaled_weight > table->rules_limit) {
        table->rule_violation_flag = true;
        printf("   [Yul CPU Rule Block] Rejected update of %lu weight on Slot %u (Exceeds limit %lu)\n", 
               scaled_weight, tdma->active_slot, table->rules_limit);
    } else {
        table->accumulated_value += scaled_weight;
        table->rule_violation_flag = false;
        printf("   [Yul CPU Accumu] Accumulated %lu weight on Slot %u (Total: %lu)\n", 
               scaled_weight, tdma->active_slot, table->accumulated_value);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT YUL CPU CYCLE ACCUMULATOR AND TDMA RULES\n");
    printf("=============================================================\n");

    TdmaState tdma = {
        .active_slot = 1,
        .slot_duration_ms = BASE_SLOT_DURATION, // 50ms
        .clock_drift_ns = 5                    // 5ns positive drift
    };

    AccumulatorTable table = {
        .accumulated_value = 0,
        .rules_limit = MAX_ACCUMULATOR_LIMIT,
        .rule_violation_flag = false
    };

    // 1. Test clean accumulation (scaled weight + drift compensation)
    // base_weight = 10 -> scaled = 10 * (50/10) = 50 -> drift comp (+5) = 55
    printf("1. Simulating clean cycle accumulation...\n");
    accumulate_yul_cycle(&tdma, &table, 10);
    assert(table.accumulated_value == 55);
    assert(table.rule_violation_flag == false);
    printf("   ✓ Accumulation succeeded. value = %lu\n\n", table.accumulated_value);

    // 2. Test rules validation limit breach
    // Attempting to accumulate large transaction (weight = 200) -> scaled = 1000 + 5 = 1005 (exceeds limit 1000)
    printf("2. Simulating rules limit breach rejection...\n");
    accumulate_yul_cycle(&tdma, &table, 200);
    assert(table.accumulated_value == 55); // Value should remain unchanged
    assert(table.rule_violation_flag == true);
    printf("   ✓ Rules-based preemption validation verified.\n\n");

    printf("=============================================================\n");
    printf("YUL CPU ACCUMULATOR & TDMA INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
