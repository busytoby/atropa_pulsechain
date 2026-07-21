#ifndef TSFI_AUTODIN_CUMULATIVE_CONTRACT_RESOLVER_H
#define TSFI_AUTODIN_CUMULATIVE_CONTRACT_RESOLVER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char dynamic_contract_address_identifier[64]; // dynamic_<address> - Rule 9
    uint64_t contract_base_register_value;
    uint64_t contract_signal_register_value;
    uint64_t contract_channel_register_value;
    uint64_t contract_pole_register_value;
    uint64_t contract_foundation_register_value;
    uint64_t contract_dynamo_register_value;
    uint64_t contract_monopole_register_value;
} autodin_cumulative_contract_state_t;

typedef struct {
    autodin_cumulative_contract_state_t registered_contracts[16];
    uint32_t total_registered_contracts;
} autodin_cumulative_contract_resolver_t;

/* Initialize Automated Digital Network cumulative contract resolver */
int autodin_cumulative_contract_resolver_initialize(
    autodin_cumulative_contract_resolver_t *resolver
);

/* Resolve contract state on ZMM VM strictly via address-based format (dynamic_<address> - Rule 9) */
int autodin_cumulative_contract_resolver_resolve_by_address(
    autodin_cumulative_contract_resolver_t *resolver,
    const char *contract_address,
    autodin_cumulative_contract_state_t **contract_state_output
);

/* Register a new dynamic contract instance onto ZMM VM state map */
int autodin_cumulative_contract_resolver_register_contract(
    autodin_cumulative_contract_resolver_t *resolver,
    const char *contract_address,
    uint64_t base_value,
    uint64_t signal_value
);

#endif // TSFI_AUTODIN_CUMULATIVE_CONTRACT_RESOLVER_H
