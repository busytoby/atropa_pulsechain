#ifndef AUNCIENT_CANARY_TRIPWIRE_FATAL_EXIT_THEOREMS_2136_2140_H
#define AUNCIENT_CANARY_TRIPWIRE_FATAL_EXIT_THEOREMS_2136_2140_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CANARY_SEVERITY_HEALTHY = 0,
    CANARY_SEVERITY_WARNING = 1,
    CANARY_SEVERITY_FATAL_EXIT = 2
} CanarySeverityLevel;

typedef struct {
    uint32_t channel_id;
    uint64_t expected_canary;
    uint64_t observed_canary;
    CanarySeverityLevel severity;
    bool is_warning_emitted;
    bool is_fatal_exit_triggered;
    char panic_message[64];
} CanaryTripwireAuditRecord;

typedef struct {
    CanaryTripwireAuditRecord audit_log[16];
    uint32_t total_audits_performed;
    uint32_t warning_events_count;
    uint32_t fatal_exits_triggered;
    bool is_tripwire_monitor_active;
    bool is_system_state_frozen_safely;
} CanaryTripwireFatalMonitorContext;

typedef struct {
    float in_silicon_canary_monitor_fidelity;
    float canary_monitor_strategy_datbin_merkle_ratio;
    float canary_audit_latency_ns;
    uint64_t verified_canary_monitor_saat_clearances;

    bool canary_fatal_monitor_verified;
    bool canary_monitor_strategy_merkle_verified;
    bool canary_monitor_submicro_latency_verified;
    bool canary_monitor_lossless_saat_verified;
    bool sovereign_2140_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} CanaryTripwireFatalBeyond2135State;

int cpm_tomie_canary_monitor_init(CanaryTripwireFatalMonitorContext *ctx);
int cpm_tomie_canary_audit_slice(CanaryTripwireFatalMonitorContext *ctx, uint32_t channel_id, uint64_t observed_val, uint64_t expected_val, bool force_fatal);
void auncient_canary_tripwire_fatal_init(CanaryTripwireFatalBeyond2135State *state);
bool auncient_canary_tripwire_fatal_verify_theorems_2136_2140(CanaryTripwireFatalBeyond2135State *state);
uint32_t auncient_canary_tripwire_fatal_compute_rule18(const CanaryTripwireFatalBeyond2135State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_CANARY_TRIPWIRE_FATAL_EXIT_THEOREMS_2136_2140_H */
