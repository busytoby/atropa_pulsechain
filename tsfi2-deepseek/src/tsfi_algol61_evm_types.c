#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_algol61_evm_types.h"
#include "tsfi_lowpower_fet.h"

int tsfi_algol61_evm_type_registry_init(
    uint32_t registry_id,
    tsfi_algol61_evm_type_registry_t *registry
) {
    if (!registry) return -1;
    memset(registry, 0, sizeof(tsfi_algol61_evm_type_registry_t));

    registry->registry_id = registry_id;
    registry->evm_gas_units = 260; // 260 Gas / Auncient Ether Units per map/uint256 operation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    registry->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(registry->tape_dat_bin, sizeof(registry->tape_dat_bin), "ALGOL61_EVM_TYPES_%08X.DAT.BIN", registry_id);
    return 0;
}

int tsfi_algol61_push_uint256(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t val_256[32]
) {
    if (!registry || !val_256 || registry->stack_pointer >= 32) return -1;
    algol61_evm_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = ALGOL61_TYPE_UINT256;
    w->status_flags = 0x0001;
    memcpy(w->payload, val_256, 32);
    return 0;
}

int tsfi_algol61_push_address(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t addr_160[20]
) {
    if (!registry || !addr_160 || registry->stack_pointer >= 32) return -1;
    algol61_evm_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = ALGOL61_TYPE_ADDRESS;
    w->status_flags = 0x0001;
    memset(w->payload, 0, 12); // Padded upper 12 bytes
    memcpy(w->payload + 12, addr_160, 20);
    return 0;
}

int tsfi_algol61_push_bytes32(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t bytes_32[32]
) {
    if (!registry || !bytes_32 || registry->stack_pointer >= 32) return -1;
    algol61_evm_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = ALGOL61_TYPE_BYTES32;
    w->status_flags = 0x0001;
    memcpy(w->payload, bytes_32, 32);
    return 0;
}

int tsfi_algol61_map_set(
    tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t key[32],
    const uint8_t value[32]
) {
    if (!registry || !key || !value) return -1;

    // Check if key already exists
    for (size_t i = 0; i < registry->map_count; i++) {
        if (memcmp(registry->map_storage[i].key, key, 32) == 0) {
            memcpy(registry->map_storage[i].value, value, 32);
            return 0;
        }
    }

    if (registry->map_count >= 16) return -1;
    algol61_map_entry_t *entry = &registry->map_storage[registry->map_count++];
    memcpy(entry->key, key, 32);
    memcpy(entry->value, value, 32);

    printf("[ALGOL61 MAP SET] Reg ID: %u | Map Key[0..3]: %02X%02X%02X%02X | Count: %zu | Gas: %u | File: %s\n",
           registry->registry_id, key[0], key[1], key[2], key[3], registry->map_count, registry->evm_gas_units, registry->tape_dat_bin);

    return 0;
}

int tsfi_algol61_map_get(
    const tsfi_algol61_evm_type_registry_t *registry,
    const uint8_t key[32],
    uint8_t out_value[32]
) {
    if (!registry || !key || !out_value) return -1;

    for (size_t i = 0; i < registry->map_count; i++) {
        if (memcmp(registry->map_storage[i].key, key, 32) == 0) {
            memcpy(out_value, registry->map_storage[i].value, 32);
            printf("[ALGOL61 MAP GET] Reg ID: %u | Map Key[0..3]: %02X%02X%02X%02X -> Val[0..3]: %02X%02X%02X%02X\n",
                   registry->registry_id, key[0], key[1], key[2], key[3], out_value[0], out_value[1], out_value[2], out_value[3]);
            return 0; // Found
        }
    }

    return -1; // Not found
}
