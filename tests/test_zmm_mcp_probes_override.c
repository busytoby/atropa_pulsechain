#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_DIAGNOSTIC_PROBES 4

typedef double (*DiagnosticProbeFn)(void);

typedef struct {
    char name[32];
    DiagnosticProbeFn fn;
    bool is_active;
} DiagnosticProbeEntry;

DiagnosticProbeEntry g_probe_registry[MAX_DIAGNOSTIC_PROBES];
size_t g_probe_count = 0;

// Test probe functions
double probe_loop_resistance(void) {
    return 50.4; // 50.4 Ohms simulated ground loop resistance
}

double probe_clock_jitter(void) {
    return 0.12; // 0.12 ns simulated jitter
}

// Register a new diagnostic probe
void register_zmm_diagnostic_probe(const char *name, DiagnosticProbeFn fn) {
    if (g_probe_count < MAX_DIAGNOSTIC_PROBES) {
        DiagnosticProbeEntry *entry = &g_probe_registry[g_probe_count++];
        strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->fn = fn;
        entry->is_active = true;
        printf("   [Probe Registry] Registered diagnostic probe: %s\n", name);
    }
}

// Execute all active diagnostic probes
void run_zmm_diagnostic_probes(void) {
    for (size_t i = 0; i < g_probe_count; i++) {
        if (g_probe_registry[i].is_active && g_probe_registry[i].fn) {
            double metric = g_probe_registry[i].fn();
            printf("   [Diagnostics Probe Run] Probe: %s -> Output: %.2f\n",
                   g_probe_registry[i].name, metric);
        }
    }
}

// Central dispatcher state
typedef struct {
    bool is_locked_out;
    uint32_t consecutive_failures;
} DispatcherState;

DispatcherState g_dispatcher = { false, 0 };

// Ingress transaction dispatch verifying command override and running diagnostics
bool dispatch_zmm_transaction(const char *command, bool is_admin_override, DispatcherState *state, const char **out_err) {
    // 1. Run all registerable diagnostics probes before dispatching command
    run_zmm_diagnostic_probes();

    // 2. Process Admin Override bypass check
    if (is_admin_override) {
        state->is_locked_out = false;
        state->consecutive_failures = 0;
        printf("   [OVERRIDE] Master administrator override signature detected! Lockout cleared.\n");
        return true; // Execution permitted
    }

    if (state->is_locked_out) {
        *out_err = "REVERT: DISPATCH_BLOCKED_BY_ACTIVE_LOCKOUT";
        return false;
    }

    if (strcmp(command, "VALID_TX") != 0) {
        state->consecutive_failures++;
        if (state->consecutive_failures >= 3) {
            state->is_locked_out = true;
        }
        *out_err = "REVERT: TRANSACTION_VALIDATION_FAILED";
        return false;
    }

    state->consecutive_failures = 0;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: DISPATCH PROBES AND OVERRIDE TESTS\n");
    printf("=============================================================\n");

    // 1. Register diagnostic probes
    printf("1. Registering active system diagnostic probes...\n");
    register_zmm_diagnostic_probe("LoopResistance", probe_loop_resistance);
    register_zmm_diagnostic_probe("ClockJitter", probe_clock_jitter);
    assert(g_probe_count == 2);
    printf("   ✓ Diagnostic probes registered.\n\n");

    const char *err = NULL;

    // 2. Trigger lockout via successive failures
    printf("2. Simulating successive validation failures to trigger lockout...\n");
    bool res = dispatch_zmm_transaction("BAD_TX", false, &g_dispatcher, &err);
    assert(res == false);
    res = dispatch_zmm_transaction("BAD_TX", false, &g_dispatcher, &err);
    assert(res == false);
    res = dispatch_zmm_transaction("BAD_TX", false, &g_dispatcher, &err);
    assert(res == false);
    assert(g_dispatcher.is_locked_out == true);
    printf("   ✓ Consecutive failures triggered lockout state successfully.\n\n");

    // 3. Normal transaction rejected due to active lockout
    printf("3. Sending transaction while locked out...\n");
    res = dispatch_zmm_transaction("VALID_TX", false, &g_dispatcher, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: DISPATCH_BLOCKED_BY_ACTIVE_LOCKOUT") == 0);
    printf("   ✓ Transaction blocked: %s\n\n", err);

    // 4. Admin Override clears lockout state
    printf("4. Applying administrator command override bypass...\n");
    res = dispatch_zmm_transaction("VALID_TX", true, &g_dispatcher, &err);
    assert(res == true);
    assert(g_dispatcher.is_locked_out == false);
    assert(g_dispatcher.consecutive_failures == 0);
    printf("   ✓ Lockout bypass and reset verified successfully.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT DISPATCH PROBES AND OVERRIDE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
