#ifndef TSFI_SMALGOL61_DATATYPE_STD_H
#define TSFI_SMALGOL61_DATATYPE_STD_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    SMALGOL_TYPE_INTEGER = 0x0001,
    SMALGOL_TYPE_REAL    = 0x0002,
    SMALGOL_TYPE_BOOLEAN = 0x0003,
    SMALGOL_TYPE_LABEL   = 0x0004,
    SMALGOL_TYPE_ARRAY   = 0x0005
} smalgol_type_tag_t;

typedef struct {
    uint16_t type_tag;                 // Type Tag (smalgol_type_tag_t)
    uint16_t status_flags;             // Status Flags
    union {
        int32_t  i_val;
        float    r_val;
        uint32_t b_val;
        uint32_t l_offset;
        uint32_t a_ptr;
    } payload;
} smalgol_stack_word_t;

typedef struct {
    uint32_t registry_id;              // Registry ID
    smalgol_stack_word_t stack[32];    // Executable Stack
    size_t stack_pointer;              // Stack Pointer Top
    uint32_t evm_gas_units;             // 250 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_smalgol61_datatype_registry_t;

/* Initialize SMALGOL-61 Data Type Registry & Stack */
int tsfi_smalgol61_datatype_registry_init(
    uint32_t registry_id,
    tsfi_smalgol61_datatype_registry_t *registry
);

/* Push Integer onto SMALGOL Stack */
int tsfi_smalgol61_push_int(tsfi_smalgol61_datatype_registry_t *registry, int32_t val);

/* Push Real onto SMALGOL Stack */
int tsfi_smalgol61_push_real(tsfi_smalgol61_datatype_registry_t *registry, float val);

/* Push Boolean onto SMALGOL Stack */
int tsfi_smalgol61_push_bool(tsfi_smalgol61_datatype_registry_t *registry, uint32_t val);

/* Push Label onto SMALGOL Stack */
int tsfi_smalgol61_push_label(tsfi_smalgol61_datatype_registry_t *registry, uint32_t offset);

/* Pop & Normalize Stack Word */
int tsfi_smalgol61_pop_word(
    tsfi_smalgol61_datatype_registry_t *registry,
    smalgol_stack_word_t *out_word
);

#endif // TSFI_SMALGOL61_DATATYPE_STD_H
