#ifndef TSFI_SMALGOL61_ENGINE_H
#define TSFI_SMALGOL61_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t process_id;           // Process ID
    uint32_t program_counter;      // Program Counter PC
    int64_t registers[16];         // SMALGOL-61 Integer Registers (R0..R15)
    uint32_t evm_gas_units;        // 290 Gas / Auncient Ether Units
    double fet_power_watts;        // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];        // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_smalgol61_proc_t;

/* Initialize a SMALGOL-61 Process Frame */
int tsfi_smalgol61_proc_init(
    uint32_t process_id,
    tsfi_smalgol61_proc_t *proc
);

/* Execute a SMALGOL-61 statement step */
int tsfi_smalgol61_proc_step(
    tsfi_smalgol61_proc_t *proc,
    const char *smalgol_statement
);

#endif // TSFI_SMALGOL61_ENGINE_H
