#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define OBS_LOG_SIZE 8

typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// 1. Sysop Observation Log (Pair ID tracking)
typedef struct {
    PPN peer;
    char activity[32];
    uint64_t cycle;
} SysopObservation;

SysopObservation g_observation_log[OBS_LOG_SIZE];
size_t g_obs_count = 0;

void record_sysop_observation(PPN peer, const char *activity, uint64_t cycle) {
    size_t idx = g_obs_count % OBS_LOG_SIZE;
    g_observation_log[idx].peer = peer;
    strncpy(g_observation_log[idx].activity, activity, sizeof(g_observation_log[idx].activity) - 1);
    g_observation_log[idx].cycle = cycle;
    g_obs_count++;
    printf("   [Sysop Observe] Cycle %lu: PPN [%u,%u] performed '%s'\n",
           cycle, peer.project, peer.programmer, activity);
}

// 2. Sysop Command Injection
typedef struct {
    PPN target_peer;
    char injected_command[32];
    bool pending;
} SysopInjection;

SysopInjection g_injection = { {0, 0}, "", false };

void inject_sysop_command(PPN target, const char *command) {
    g_injection.target_peer = target;
    strncpy(g_injection.injected_command, command, sizeof(g_injection.injected_command) - 1);
    g_injection.pending = true;
    printf("   [Sysop Inject] Scheduled command injection '%s' for PPN [%u,%u]\n",
           command, target.project, target.programmer);
}

// 3. AI Automation Engine
bool g_automation_mitigation_active = false;

void evaluate_ai_automation_rules(const char *last_observation) {
    // If telemetry detects anomalous patterns (e.g. memory boundary breaches), trigger automation
    if (strstr(last_observation, "MEMORY_BREACH") != NULL) {
        g_automation_mitigation_active = true;
        printf("   [AI Automation] ALERT: Anomaly detected! Mitigating memory space leakage...\n");
    }
}

// 4. Unified Yul CPU Cycle Loop (Per-cycle evaluation)
typedef struct {
    uint64_t current_cycle;
    bool peer_locked_out;
} YulCpuState;

void execute_yul_cpu_cycle(YulCpuState *cpu, PPN active_peer, const char *action) {
    cpu->current_cycle++;

    // A. Perform Sysop Observation check
    record_sysop_observation(active_peer, action, cpu->current_cycle);

    // B. Evaluate AI Automation metrics based on observation
    evaluate_ai_automation_rules(action);

    // C. Handle Sysop Command Injection if pending for this peer
    if (g_injection.pending &&
        g_injection.target_peer.project == active_peer.project &&
        g_injection.target_peer.programmer == active_peer.programmer) {
        
        printf("   [Yul Execution] Injected command '%s' dispatched successfully on cycle %lu.\n",
               g_injection.injected_command, cpu->current_cycle);
        g_injection.pending = false;

        // Admin override clears lockout
        if (strcmp(g_injection.injected_command, "CLEAR_LOCKOUT") == 0) {
            cpu->peer_locked_out = false;
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: UNIFIED YUL CPU CYCLE CONTROLLER TESTS\n");
    printf("=============================================================\n");

    YulCpuState cpu = { .current_cycle = 0, .peer_locked_out = true };
    PPN peer = { 10, 5 };

    // 1. Cycle 1: Perform normal user action (Logged by Sysop observer)
    printf("1. Running cycle 1: Normal action...\n");
    execute_yul_cpu_cycle(&cpu, peer, "READ_REGISTER");
    assert(g_obs_count == 1);
    assert(strcmp(g_observation_log[0].activity, "READ_REGISTER") == 0);
    printf("   ✓ Sysop observation recorded.\n\n");

    // 2. Cycle 2: Anomalous memory action (Triggers AI automated mitigation)
    printf("2. Running cycle 2: Anomaly action...\n");
    execute_yul_cpu_cycle(&cpu, peer, "MEMORY_BREACH_DETECTED");
    assert(g_automation_mitigation_active == true);
    printf("   ✓ AI Automation trigger verified.\n\n");

    // 3. Cycle 3: Sysop Injection (Clear lockout state)
    printf("3. Scheduling Sysop injection and running cycle 3...\n");
    inject_sysop_command(peer, "CLEAR_LOCKOUT");
    assert(cpu.peer_locked_out == true);

    execute_yul_cpu_cycle(&cpu, peer, "POLL_DEVICE");
    assert(cpu.peer_locked_out == false); // Lockout cleared by injected command
    assert(g_injection.pending == false);
    printf("   ✓ Sysop command injection executed on-cycle successfully.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT UNIFIED YUL CPU CYCLE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
