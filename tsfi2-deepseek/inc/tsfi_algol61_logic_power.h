#ifndef TSFI_ALGOL61_LOGIC_POWER_H
#define TSFI_ALGOL61_LOGIC_POWER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ALGOL61_GATE_NOT = 0,
    ALGOL61_GATE_AND,
    ALGOL61_GATE_OR,
    ALGOL61_GATE_XOR
} tsfi_algol61_gate_type_t;

typedef struct {
    tsfi_algol61_gate_type_t gate_type;
    uint8_t input_a;
    uint8_t input_b;
    uint8_t output_val;
    double v_gate_discharge; // Verlet soft-body gate voltage (3.3V floor)
    double fet_power_watts;  // Power draw in Watts (0.0109 W)
    uint32_t evm_gas_cost;   // EVM Gas Billing Slot (50..150 Gas)
    char tape_dat_bin[128];  // Rule 13 Filename (.DAT.BIN)
} tsfi_algol61_logic_summary_t;

/* Evaluate ALGOL 61 logic gate with FET discharge physics solver */
int tsfi_algol61_eval_logic_gate(
    tsfi_algol61_gate_type_t gate_type,
    uint8_t input_a,
    uint8_t input_b,
    tsfi_algol61_logic_summary_t *out_summary
);

#endif // TSFI_ALGOL61_LOGIC_POWER_H
