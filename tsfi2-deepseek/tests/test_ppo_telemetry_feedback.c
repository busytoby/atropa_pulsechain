#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    float pressure;
    float frequency;
    int disconnect_penalty;
    char suggestion[128];
} PPOFeedback;

void tsfi_ppo_telemetry_event(int friction_units, PPOFeedback *out) {
    if (friction_units > 0) {
        out->pressure = 0.5f + (float)friction_units * 0.05f;
        out->frequency = 40.0f + (float)friction_units * 10.0f;
        out->disconnect_penalty = friction_units;
        snprintf(out->suggestion, 128, "FRACTURE DETECTED. Switched to Shadow Rigidity Pipeline.");
    } else {
        out->pressure = 0.05f;
        out->frequency = 40.0f;
        out->disconnect_penalty = 0;
        snprintf(out->suggestion, 128, "RIGID. Strategic Velocity Optimal.");
    }
}

int main() {
    printf("=== TSFi Unit Test: PPO Telemetry Feedback Loop ===\n");

    PPOFeedback feedback;

    // Scenario 1: Clean State
    printf("[AUDIT] Simulating 0 friction events...\n");
    tsfi_ppo_telemetry_event(0, &feedback);
    printf("  -> Pressure: %.2f | Freq: %.1f Hz\n", feedback.pressure, feedback.frequency);
    printf("  -> PPO Suggestion: %s\n", feedback.suggestion);
    if (feedback.pressure > 0.1f) { printf("[FAIL] Pressure too high for clean state.\n"); return 1; }

    // Scenario 2: Friction State (3 Warnings)
    printf("\n[AUDIT] Simulating 3 friction events...\n");
    tsfi_ppo_telemetry_event(3, &feedback);
    printf("  -> Pressure: %.2f | Freq: %.1f Hz\n", feedback.pressure, feedback.frequency);
    printf("  -> PPO Suggestion: %s\n", feedback.suggestion);
    printf("  -> Penalty: -%d DISCONNECT\n", feedback.disconnect_penalty);
    if (feedback.pressure < 0.6f) { printf("[FAIL] Pressure too low for friction state.\n"); return 1; }

    printf("\n[SUCCESS] PPO Telemetry Feedback Verified.\n");
    return 0;
}
