#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define STACK_CAPACITY 64
#define PAYLOAD_SIZE 64
#define LATENCY_LIMIT_NS 150000
#define MAX_TASKS 4
#define MAX_MEMBERS 4

// vppd.bin ABI definition
typedef enum {
    ABI_CMD_PING = 1,
    ABI_CMD_STATUS = 2,
    ABI_CMD_ROUTE = 3
} vppd_abi_cmd_t;

typedef struct {
    uint64_t sequence;
    uint32_t precedence;
    char operation_cmd[64];
} vppd_packet_t;

// Self-contained execution stack with checkpoint/rollback capability
typedef struct {
    int64_t data[STACK_CAPACITY];
    int top;
    int64_t checkpoint_data[STACK_CAPACITY];
    int checkpoint_top;
} vppd_stack_t;

// vppd.bin main executable context
typedef struct {
    vppd_stack_t stack;
    uint64_t total_packets;
    double pll_phase_error;
    uint64_t tsc_cycles;

    // Dynamic tasks spawned from CICS
    char active_tasks[MAX_TASKS][160];
    int task_count;

    // Dynamic schema members added from CICS
    char dynamic_members[MAX_MEMBERS][64];
    int64_t dynamic_values[MAX_MEMBERS];
    int member_count;

    // SVDAG State Vector registers (State parameters)
    int64_t svdag_base;
    int64_t svdag_signal;
    int64_t checkpoint_svdag_base;
    int64_t checkpoint_svdag_signal;
} vppd_context_t;

// Stack helpers
static void push_stack(vppd_stack_t *s, int64_t val) {
    if (s->top < STACK_CAPACITY) {
        s->data[s->top++] = val;
    }
}

static int64_t pop_stack(vppd_stack_t *s) {
    if (s->top > 0) {
        return s->data[--s->top];
    }
    return 0;
}

// Saves current stack state and SVDAG registers to the checkpoint
static void save_checkpoint_integrated(vppd_context_t *ctx) {
    // 1. Stack checkpoint
    ctx->stack.checkpoint_top = ctx->stack.top;
    memcpy(ctx->stack.checkpoint_data, ctx->stack.data, sizeof(int64_t) * ctx->stack.top);

    // 2. SVDAG checkpoint
    ctx->checkpoint_svdag_base = ctx->svdag_base;
    ctx->checkpoint_svdag_signal = ctx->svdag_signal;
}

// Restores stack state and SVDAG registers to the saved checkpoint
static void rollback_checkpoint_integrated(vppd_context_t *ctx) {
    // 1. Stack rollback
    ctx->stack.top = ctx->stack.checkpoint_top;
    memcpy(ctx->stack.data, ctx->stack.checkpoint_data, sizeof(int64_t) * ctx->stack.checkpoint_top);

    // 2. SVDAG rollback
    ctx->svdag_base = ctx->checkpoint_svdag_base;
    ctx->svdag_signal = ctx->checkpoint_svdag_signal;
}

// Commits the active stack and SVDAG transitions
static void commit_checkpoint_integrated(vppd_context_t *ctx) {
    ctx->stack.checkpoint_top = ctx->stack.top;
    ctx->checkpoint_svdag_base = ctx->svdag_base;
    ctx->checkpoint_svdag_signal = ctx->svdag_signal;
}

// CICS ASCII Terminal interface rendering
static void render_cics_terminal(const vppd_context_t *ctx) {
    printf("\n========================================\n");
    printf("--- CICS TERMINAL DIAGNOSTIC SCREEN ---\n");
    printf("  VPPD STATUS    : ACTIVE (Self-Contained)\n");
    printf("  PACKETS SWITCHED: %lu\n", ctx->total_packets);
    printf("  PLL PHASE ERROR: %.4f\n", ctx->pll_phase_error);
    printf("  SYSTEM CYCLES  : %lu\n", ctx->tsc_cycles);
    printf("  STACK DEPTH    : %d\n", ctx->stack.top);
    printf("  SVDAG REGISTERS: Base=%ld, Signal=%ld\n", ctx->svdag_base, ctx->svdag_signal);
    
    // Display active spawned tasks
    printf("  SPAWNED TASKS  : %d\n", ctx->task_count);
    for (int i = 0; i < ctx->task_count; i++) {
        printf("    [%d] %s\n", i, ctx->active_tasks[i]);
    }

    // Display dynamic schema members
    printf("  DYNAMIC MEMBERS: %d\n", ctx->member_count);
    for (int i = 0; i < ctx->member_count; i++) {
        printf("    -> %s = %ld\n", ctx->dynamic_members[i], ctx->dynamic_values[i]);
    }
    printf("========================================\n");
    fflush(stdout);
}

// Helper to read nanosecond clock cycles
static uint64_t read_tsc_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// Self-contained ABI entry point
bool vppd_abi_entry(vppd_context_t *ctx, vppd_abi_cmd_t cmd, const vppd_packet_t *pkt, char *out_status) {
    uint64_t start_time = read_tsc_ns();
    ctx->tsc_cycles += 50; // Emulate internal cycles

    switch (cmd) {
        case ABI_CMD_PING:
            strcpy(out_status, "ABI_PONG");
            break;
        case ABI_CMD_STATUS:
            sprintf(out_status, "OK;packets=%lu;phase=%.4f;tasks=%d", ctx->total_packets, ctx->pll_phase_error, ctx->task_count);
            break;
        case ABI_CMD_ROUTE:
            if (!pkt) return false;
            ctx->total_packets++;
            
            // Push mock operation payload value to execution stack
            push_stack(&(ctx->stack), pkt->sequence);
            
            // Perform simulated execution on stack
            int64_t val = pop_stack(&(ctx->stack));
            push_stack(&(ctx->stack), val * 2);
            
            sprintf(out_status, "ROUTED;seq=%lu;stack_top=%ld", pkt->sequence, ctx->stack.data[ctx->stack.top - 1]);
            break;
        default:
            strcpy(out_status, "ABI_UNKNOWN");
            return false;
    }

    uint64_t duration = read_tsc_ns() - start_time;
    // Self-Profiling Verification Assert
    assert(duration < LATENCY_LIMIT_NS);
    return true;
}

// Interactive CICS Command to load and parse a .pli source task file
static void cics_load_pli_file(vppd_context_t *ctx, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Result: Error. Cannot open file %s\n", filename);
        return;
    }

    char line[256];
    int parsed_procedures = 0;
    printf("Result: Loading and parsing PL/I source file...\n");

    while (fgets(line, sizeof(line), f)) {
        // Look for PROCEDURE declaration tokens in PL/I
        if (strstr(line, "PROCEDURE") != NULL) {
            // Find procedure name
            char proc_name[64];
            memset(proc_name, 0, sizeof(proc_name));
            char *colon = strchr(line, ':');
            if (colon) {
                size_t name_len = colon - line;
                if (name_len > 63) name_len = 63;
                strncpy(proc_name, line, name_len);
                // Strip whitespace
                proc_name[name_len] = '\0';
                
                // Add to active task list
                if (ctx->task_count < MAX_TASKS) {
                    snprintf(ctx->active_tasks[ctx->task_count++], 160, "PLI_PROC_%s", proc_name);
                    parsed_procedures++;
                }
            }
        }
    }
    fclose(f);
    ctx->tsc_cycles += (parsed_procedures * 200);
    printf("Result: Parsed and registered %d PL/I procedures.\n", parsed_procedures);
}

// Interactive CICS Terminal console loop
static void run_interactive_mode(vppd_context_t *ctx) {
    char input_line[128];
    char abi_buffer[256];

    printf("Entering VPPD Interactive Mode. Type 'help' for commands.\n");
    fflush(stdout);

    while (1) {
        render_cics_terminal(ctx);
        printf("vppd> ");
        fflush(stdout);

        if (!fgets(input_line, sizeof(input_line), stdin)) {
            break; // EOF
        }

        // Clean newline
        input_line[strcspn(input_line, "\r\n")] = 0;

        if (strcmp(input_line, "exit") == 0 || strcmp(input_line, "quit") == 0) {
            printf("Exiting Interactive Mode.\n");
            fflush(stdout);
            break;
        } else if (strcmp(input_line, "help") == 0) {
            printf("Commands:\n");
            printf("  ping                       - Query daemon responsiveness\n");
            printf("  status                     - Query current telemetry statistics\n");
            printf("  route <N>                  - Route a simulated transaction packet with sequence N\n");
            printf("  spawn <name> <clearance>   - Spawn a dynamic contract process task\n");
            printf("  add-member <name> <v> <m>  - Add a dynamic data member with val <v> and signature mask <m>\n");
            printf("  load-pli <filename>        - Load and parse a PL/I source task file\n");
            printf("  set-svdag <base> <signal>  - Set SVDAG state vector registers\n");
            printf("  checkpoint                 - Save stack and SVDAG checkpoints\n");
            printf("  rollback                   - Rollback stack and SVDAG to saved checkpoint\n");
            printf("  commit                     - Commit active changes\n");
            printf("  exit                       - Terminate the session\n");
            fflush(stdout);
        } else if (strcmp(input_line, "ping") == 0) {
            vppd_abi_entry(ctx, ABI_CMD_PING, NULL, abi_buffer);
            printf("Result: %s\n", abi_buffer);
            fflush(stdout);
        } else if (strcmp(input_line, "status") == 0) {
            vppd_abi_entry(ctx, ABI_CMD_STATUS, NULL, abi_buffer);
            printf("Result: %s\n", abi_buffer);
            fflush(stdout);
        } else if (strcmp(input_line, "checkpoint") == 0) {
            save_checkpoint_integrated(ctx);
            printf("Result: Checkpoint saved successfully.\n");
            fflush(stdout);
        } else if (strcmp(input_line, "rollback") == 0) {
            rollback_checkpoint_integrated(ctx);
            printf("Result: Rollback completed successfully.\n");
            fflush(stdout);
        } else if (strcmp(input_line, "commit") == 0) {
            commit_checkpoint_integrated(ctx);
            printf("Result: Commit completed successfully.\n");
            fflush(stdout);
        } else if (strncmp(input_line, "load-pli ", 9) == 0) {
            cics_load_pli_file(ctx, input_line + 9);
            fflush(stdout);
        } else if (strncmp(input_line, "set-svdag ", 10) == 0) {
            int64_t b = 0, s = 0;
            if (sscanf(input_line + 10, "%ld %ld", &b, &s) == 2) {
                ctx->svdag_base = b;
                ctx->svdag_signal = s;
                printf("Result: SVDAG registers updated.\n");
            } else {
                printf("Usage: set-svdag <base> <signal>\n");
            }
            fflush(stdout);
        } else if (strncmp(input_line, "route ", 6) == 0) {
            uint64_t seq = strtoull(input_line + 6, NULL, 10);
            vppd_packet_t pkt;
            pkt.sequence = seq;
            pkt.precedence = 3; // FLASH
            strcpy(pkt.operation_cmd, "EXEC");
            
            vppd_abi_entry(ctx, ABI_CMD_ROUTE, &pkt, abi_buffer);
            printf("Result: %s\n", abi_buffer);
            fflush(stdout);
        } else if (strncmp(input_line, "spawn ", 6) == 0) {
            char name[64];
            char clearance[64];
            if (sscanf(input_line + 6, "%63s %63s", name, clearance) == 2) {
                if (ctx->task_count < MAX_TASKS) {
                    snprintf(ctx->active_tasks[ctx->task_count++], 160, "%s(%s)", name, clearance);
                    ctx->tsc_cycles += 100;
                    printf("Result: Task spawned successfully.\n");
                } else {
                    printf("Result: Spawn failed. Maximum task capacity reached.\n");
                }
            } else {
                printf("Usage: spawn <name> <clearance>\n");
            }
            fflush(stdout);
        } else if (strncmp(input_line, "add-member ", 11) == 0) {
            char name[64];
            int64_t val = 0;
            uint32_t mask = 0;
            if (sscanf(input_line + 11, "%63s %ld %u", name, &val, &mask) == 3) {
                // Count signatures in mask (Ackerman communal proof threshold check)
                int signatures = 0;
                for (int i = 0; i < 32; i++) {
                    if ((mask >> i) & 1) {
                        signatures++;
                    }
                }
                if (signatures >= 3) {
                    if (ctx->member_count < MAX_MEMBERS) {
                        snprintf(ctx->dynamic_members[ctx->member_count], 64, "%s", name);
                        ctx->dynamic_values[ctx->member_count] = val;
                        ctx->member_count++;
                        ctx->tsc_cycles += 150;
                        printf("Result: Dynamic schema member added successfully.\n");
                    } else {
                        printf("Result: Expansion failed. Member capacity reached.\n");
                    }
                } else {
                    printf("Result: Ackerman Quorum failed. Insufficient signatures (%d/3 required).\n", signatures);
                }
            } else {
                printf("Usage: add-member <name> <value> <signature_mask>\n");
            }
            fflush(stdout);
        } else if (strlen(input_line) > 0) {
            printf("Unknown command: %s\n", input_line);
            fflush(stdout);
        }
    }
}

int main(int argc, char **argv) {
    // Instantiate self-contained vppd context
    vppd_context_t vppd;
    memset(&vppd, 0, sizeof(vppd_context_t));
    vppd.pll_phase_error = 0.002;
    vppd.tsc_cycles = 10000;
    vppd.svdag_base = 500;
    vppd.svdag_signal = 1200;

    // Check if interactive flag is provided
    if (argc > 1 && (strcmp(argv[1], "--interactive") == 0 || strcmp(argv[1], "-i") == 0)) {
        run_interactive_mode(&vppd);
        return 0;
    }

    printf("=============================================================\n");
    printf("AUNCIENT VPPD.BIN SELF-CONTAINED EXECUTION & PROFILING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    char abi_buffer[256];

    // 1. Test ABI Ping
    printf("[TEST] Querying ABI: PING...\n");
    fflush(stdout);
    bool ok = vppd_abi_entry(&vppd, ABI_CMD_PING, NULL, abi_buffer);
    assert(ok == true);
    assert(strcmp(abi_buffer, "ABI_PONG") == 0);
    printf("   ✓ ABI Response: %s\n", abi_buffer);
    fflush(stdout);

    // 2. Test ABI Route with stack execution
    printf("[TEST] Routing transaction packet through ABI with stack evaluation...\n");
    fflush(stdout);
    vppd_packet_t pkt = {
        .sequence = 8080,
        .precedence = 3, // FLASH
        .operation_cmd = "EXEC"
    };

    ok = vppd_abi_entry(&vppd, ABI_CMD_ROUTE, &pkt, abi_buffer);
    assert(ok == true);
    assert(strstr(abi_buffer, "ROUTED;seq=8080") != NULL);
    printf("   ✓ ABI Response: %s\n", abi_buffer);
    fflush(stdout);

    // 3. Render CICS ASCII Terminal
    render_cics_terminal(&vppd);

    // 4. Test self-profiling stats query
    printf("[TEST] Querying self-profiling status...\n");
    fflush(stdout);
    ok = vppd_abi_entry(&vppd, ABI_CMD_STATUS, NULL, abi_buffer);
    assert(ok == true);
    assert(strstr(abi_buffer, "packets=1") != NULL);
    printf("   ✓ ABI Response: %s\n", abi_buffer);
    fflush(stdout);

    // 5. Test Integrated SVDAG Rollback
    printf("[TEST] Verifying integrated SVDAG rollback...\n");
    fflush(stdout);
    save_checkpoint_integrated(&vppd);
    
    // Mutate state registers
    vppd.svdag_base = 9999;
    vppd.svdag_signal = 8888;
    
    // Perform rollback
    rollback_checkpoint_integrated(&vppd);
    assert(vppd.svdag_base == 500);
    assert(vppd.svdag_signal == 1200);
    printf("   ✓ SVDAG state vector successfully restored to pre-checkpoint parameters.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VPPD.BIN SELF-PROFILING TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
