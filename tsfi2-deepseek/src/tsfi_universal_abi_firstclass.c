#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_universal_abi_firstclass.h"
#include "tsfi_lowpower_fet.h"

int tsfi_universal_abi_gateway_init(
    uint32_t gateway_id,
    tsfi_universal_abi_gateway_t *gateway
) {
    if (!gateway) return -1;
    memset(gateway, 0, sizeof(tsfi_universal_abi_gateway_t));

    gateway->gateway_id = gateway_id;
    gateway->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per first-class ABI invocation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    gateway->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(gateway->tape_dat_bin, sizeof(gateway->tape_dat_bin), "UNIVERSAL_ABI_%08X.DAT.BIN", gateway_id);
    return 0;
}

int tsfi_firstclass_abi_register(
    tsfi_universal_abi_gateway_t *gateway,
    tsfi_abi_domain_tag_t domain,
    void *native_ptr,
    tsfi_firstclass_handle_t *out_handle
) {
    if (!gateway || !out_handle || gateway->handle_count >= 32) return -1;

    tsfi_firstclass_handle_t *h = &gateway->handles[gateway->handle_count++];
    h->domain = (uint16_t)domain;
    h->flags = 0x0001; // First-Class Executable Flag
    h->handle_id = (uint64_t)gateway->handle_count;
    h->native_ptr = native_ptr;

    *out_handle = *h;

    printf("[UNIVERSAL ABI REGISTER] Gateway ID: %u | Domain: 0x%02X | Handle ID: %lu | Gas: %u | File: %s\n",
           gateway->gateway_id, domain, h->handle_id, gateway->evm_gas_units, gateway->tape_dat_bin);

    return 0;
}

int tsfi_firstclass_abi_invoke(
    tsfi_universal_abi_gateway_t *gateway,
    const tsfi_firstclass_handle_t *handle,
    uint32_t selector_id,
    const uint64_t *args,
    size_t arg_count,
    uint64_t *out_result
) {
    if (!gateway || !handle || !out_result) return -1;
    (void)args;

    // Simulate cross-domain first-class ABI call
    uint64_t result_val = 0;
    switch (handle->domain) {
        case ABI_DOMAIN_SMALLTALK: result_val = 0x5D110000 + selector_id; break;
        case ABI_DOMAIN_ALGOL61:   result_val = 0x61000000 + selector_id; break;
        case ABI_DOMAIN_COBOL:     result_val = 0xC0B00000 + selector_id; break;
        case ABI_DOMAIN_YUL_ZMM:   result_val = 0x70010000 + selector_id; break;
        case ABI_DOMAIN_VULKAN:    result_val = 0xFAFA0000 + selector_id; break;
        default:                   result_val = 0x11110000 + selector_id; break;
    }

    *out_result = result_val;

    printf("[UNIVERSAL ABI INVOKE] Domain: 0x%02X | Handle ID: %lu | Selector: 0x%08X | Args: %zu -> Result: 0x%016LX\n",
           handle->domain, handle->handle_id, selector_id, arg_count, (unsigned long long)result_val);

    return 0;
}
