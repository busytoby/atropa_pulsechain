#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_cumulative_contract_resolver.h"

#define AUTODIN_MOTZKIN_PRIME 953467954114363ULL

int autodin_cumulative_contract_resolver_initialize(
    autodin_cumulative_contract_resolver_t *resolver
) {
    if (!resolver) return -1;

    memset(resolver, 0, sizeof(autodin_cumulative_contract_resolver_t));
    return 0;
}

int autodin_cumulative_contract_resolver_register_contract(
    autodin_cumulative_contract_resolver_t *resolver,
    const char *contract_address,
    uint64_t base_value,
    uint64_t signal_value
) {
    if (!resolver || !contract_address) return -1;
    if (resolver->total_registered_contracts >= 16) return -2;

    autodin_cumulative_contract_state_t *contract = &resolver->registered_contracts[resolver->total_registered_contracts];

    // Rule 9: Address-Based Resolution (dynamic_<address>)
    if (strncmp(contract_address, "dynamic_", 8) == 0) {
        snprintf(contract->dynamic_contract_address_identifier, sizeof(contract->dynamic_contract_address_identifier), "%s", contract_address);
    } else {
        snprintf(contract->dynamic_contract_address_identifier, sizeof(contract->dynamic_contract_address_identifier), "dynamic_%s", contract_address);
    }

    contract->contract_base_register_value = base_value % AUTODIN_MOTZKIN_PRIME;
    contract->contract_signal_register_value = signal_value % AUTODIN_MOTZKIN_PRIME;

    // Derived modular fields
    contract->contract_channel_register_value = (base_value * signal_value) % AUTODIN_MOTZKIN_PRIME;
    contract->contract_pole_register_value = (base_value ^ signal_value) % AUTODIN_MOTZKIN_PRIME;
    contract->contract_foundation_register_value = (base_value + signal_value) % AUTODIN_MOTZKIN_PRIME;
    contract->contract_dynamo_register_value = (signal_value * 31ULL) % AUTODIN_MOTZKIN_PRIME;
    contract->contract_monopole_register_value = (base_value * 17ULL) % AUTODIN_MOTZKIN_PRIME;

    resolver->total_registered_contracts++;
    return 0;
}

int autodin_cumulative_contract_resolver_resolve_by_address(
    autodin_cumulative_contract_resolver_t *resolver,
    const char *contract_address,
    autodin_cumulative_contract_state_t **contract_state_output
) {
    if (!resolver || !contract_address || !contract_state_output) return -1;

    char expected_address_format[64];
    if (strncmp(contract_address, "dynamic_", 8) == 0) {
        snprintf(expected_address_format, sizeof(expected_address_format), "%s", contract_address);
    } else {
        snprintf(expected_address_format, sizeof(expected_address_format), "dynamic_%s", contract_address);
    }

    for (uint32_t index = 0; index < resolver->total_registered_contracts; index++) {
        if (strcmp(resolver->registered_contracts[index].dynamic_contract_address_identifier, expected_address_format) == 0) {
            *contract_state_output = &resolver->registered_contracts[index];
            return 0;
        }
    }

    *contract_state_output = NULL;
    return -2; // Not found
}
