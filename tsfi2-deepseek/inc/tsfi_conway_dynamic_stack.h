#ifndef TSFI_CONWAY_DYNAMIC_STACK_H
#define TSFI_CONWAY_DYNAMIC_STACK_H

#include <stdint.h>
#include <stddef.h>

#define CONWAY_MAX_STACK_DEPTH 64

typedef struct {
    uint32_t return_pc;          // Return Program Counter
    uint64_t frame_local_word;   // Local variable payload word
} tsfi_conway_stack_frame_t;

typedef struct {
    uint32_t process_id;                              // Conway Process ID
    tsfi_conway_stack_frame_t frames[CONWAY_MAX_STACK_DEPTH]; // Dynamic stack frames
    uint32_t depth;                                   // Current stack depth
    uint32_t evm_gas_units;                           // 220 Gas / Auncient Ether Units
    double fet_power_watts;                           // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                           // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_conway_dynamic_stack_t;

/* Initialize a dynamic build-as-you-go stack for a binary ledger process */
int tsfi_conway_dynamic_stack_init(
    uint32_t process_id,
    tsfi_conway_dynamic_stack_t *stack
);

/* Push a stack frame as coroutine process executes nested calls */
int tsfi_conway_dynamic_stack_push(
    tsfi_conway_dynamic_stack_t *stack,
    uint32_t return_pc,
    uint64_t local_word
);

/* Pop a stack frame upon returning from a coroutine call */
int tsfi_conway_dynamic_stack_pop(
    tsfi_conway_dynamic_stack_t *stack,
    uint32_t *out_return_pc,
    uint64_t *out_local_word
);

#endif // TSFI_CONWAY_DYNAMIC_STACK_H
