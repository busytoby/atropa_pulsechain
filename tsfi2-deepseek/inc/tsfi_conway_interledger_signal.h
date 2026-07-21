#ifndef TSFI_CONWAY_INTERLEDGER_SIGNAL_H
#define TSFI_CONWAY_INTERLEDGER_SIGNAL_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t src_process_id;       // Source Process ID
    char src_ledger_file[128];     // Source .DAT.BIN ledger filename
    uint32_t dst_process_id;       // Destination Process ID
    char dst_ledger_file[128];     // Destination .DAT.BIN ledger filename
    uint64_t signal_opcode;        // Inter-process signal opcode
    uint64_t payload_word;         // Signal payload word
    uint32_t evm_gas_units;        // 280 Gas / Auncient Ether Units
    double fet_power_watts;        // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];        // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_conway_interledger_signal_t;

/* Dispatch a signal between two running processes residing in different binary ledgers */
int tsfi_conway_dispatch_interledger_signal(
    uint32_t src_proc_id,
    const char *src_ledger,
    uint32_t dst_proc_id,
    const char *dst_ledger,
    uint64_t signal_op,
    uint64_t payload,
    tsfi_conway_interledger_signal_t *out_signal
);

#endif // TSFI_CONWAY_INTERLEDGER_SIGNAL_H
