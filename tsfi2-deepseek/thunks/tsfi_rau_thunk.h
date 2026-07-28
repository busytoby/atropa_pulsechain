#ifndef TSFI_RAU_THUNK_H
#define TSFI_RAU_THUNK_H

#include <stdint.h>

// Bridges the C environment with the Yul RAU co-processor's bulk serialization API
int tsfi_rau_thunk_dump_registers(uint32_t core_id, uint64_t *out_reg_bank_32);
int tsfi_rau_thunk_load_registers(uint32_t core_id, const uint64_t *in_reg_bank_32);

#endif // TSFI_RAU_THUNK_H
