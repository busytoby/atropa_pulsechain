#ifndef TSFI_ALGOL61_EVM_TYPES_H
#define TSFI_ALGOL61_EVM_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ALGOL61_TYPE_UINT256       = 0x0010,
    ALGOL61_TYPE_ADDRESS       = 0x0011,
    ALGOL61_TYPE_BYTES32       = 0x0012,
    ALGOL61_TYPE_MAP           = 0x0013,
    ALGOL61_TYPE_DYNAMIC_BYTES = 0x0014
} algol61_evm_type_tag_t;

typedef struct {
    uint8_t key[32];
    uint8_t value[32];
} algol61_map_entry_t;

typedef struct {
    uint16_t type_tag;
    uint16_t status_flags;
    uint8_t payload[32];               // 256-bit payload (uint256, address, bytes32)
} algol61_evm_stack_word_t;

typedef struct {
    uint32_t registry_id;
    algol61_evm_stack_word_t stack[32];
    size_t stack_pointer;
    algol61_map_entry_t map_storage[16];
    size_t map_count;
    uint32_t evm_gas_units;             // 260 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_algol61_evm_type_registry_t;

/* Initialize ALGOL-61 EVM Extended Data Type Registry */
int tsfi_algol61_evm_type_registry_init(
    uint32_t registry_id,
    tsfi_algol61_evm_type_registry_t *registry
);

/* Push uint256 onto ALGOL-61 Stack */
int tsfi_algol61_push_uint256(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t val_256[32]
);

/* Push EVM Address (20-byte padded) onto ALGOL-61 Stack */
int tsfi_algol61_push_address(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t addr_160[20]
);

/* Push bytes32 onto ALGOL-61 Stack */
int tsfi_algol61_push_bytes32(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t bytes_32[32]
);

/* Store Key-Value into ALGOL-61 EVM Map Storage */
int tsfi_algol61_map_set(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t key[32],
    const uint8_t value[32]
);

/* Fetch Value from ALGOL-61 EVM Map Storage */
int tsfi_algol61_map_get(
    const tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t key[32],
    uint8_t out_value[32]
);

#endif // TSFI_ALGOL61_EVM_TYPES_H
