#ifndef AUNCIENT_SIGNAL_DIRECTIVE_CONCURRENCY_THEOREMS_191_195_H
#define AUNCIENT_SIGNAL_DIRECTIVE_CONCURRENCY_THEOREMS_191_195_H

#include "auncient_lau_resonator_bridge.h"
#include "auncient_cpm_polyphase_compiler_executor.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_CONCURRENT_DIRECTIVES 8

typedef struct {
    uint32_t total_directives_processed;
    uint32_t deadlocks_detected;
    float    intensity_envelope_saturation;
    bool     fifo_ordering_verified;
    bool     deadlock_freedom_verified;
    bool     intensity_concurrency_verified;
    bool     saat_dividend_pipeline_verified;
    bool     concurrency_parity_verified;
    uint32_t rule18_parity_checksum;
} SignalDirectiveConcurrencyState;

void auncient_concurrency_init(SignalDirectiveConcurrencyState *state);
bool auncient_concurrency_verify_theorems_191_195(SignalDirectiveConcurrencyState *state);
uint32_t auncient_concurrency_compute_rule18(const SignalDirectiveConcurrencyState *state);

#endif /* AUNCIENT_SIGNAL_DIRECTIVE_CONCURRENCY_THEOREMS_191_195_H */
