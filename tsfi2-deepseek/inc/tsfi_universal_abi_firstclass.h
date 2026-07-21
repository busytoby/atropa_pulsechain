#ifndef TSFI_UNIVERSAL_ABI_FIRSTCLASS_H
#define TSFI_UNIVERSAL_ABI_FIRSTCLASS_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ABI_DOMAIN_SMALLTALK = 0x01,
    ABI_DOMAIN_ALGOL61   = 0x02,
    ABI_DOMAIN_COBOL     = 0x03,
    ABI_DOMAIN_YUL_ZMM   = 0x04,
    ABI_DOMAIN_VULKAN    = 0x05,
    ABI_DOMAIN_RAW_MEM   = 0x06
} tsfi_abi_domain_tag_t;

typedef struct {
    uint16_t domain;                   // Domain Tag (tsfi_abi_domain_tag_t)
    uint16_t flags;                    // Permissions & Status Flags
    uint64_t handle_id;                // Unique First-Class Handle ID
    void    *native_ptr;               // Native Memory Pointer / Struct
} tsfi_firstclass_handle_t;

typedef struct {
    uint32_t gateway_id;
    tsfi_firstclass_handle_t handles[32];
    size_t handle_count;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_universal_abi_gateway_t;

/* Initialize Universal First-Class ABI Gateway */
int tsfi_universal_abi_gateway_init(
    uint32_t gateway_id,
    tsfi_universal_abi_gateway_t *gateway
);

/* Register Universal First-Class Handle */
int tsfi_firstclass_abi_register(
    tsfi_universal_abi_gateway_t *gateway,
    tsfi_abi_domain_tag_t domain,
    void *native_ptr,
    tsfi_firstclass_handle_t *out_handle
);

/* Invoke First-Class ABI Handle */
int tsfi_firstclass_abi_invoke(
    tsfi_universal_abi_gateway_t *gateway,
    const tsfi_firstclass_handle_t *handle,
    uint32_t selector_id,
    const uint64_t *args,
    size_t arg_count,
    uint64_t *out_result
);

#endif // TSFI_UNIVERSAL_ABI_FIRSTCLASS_H
