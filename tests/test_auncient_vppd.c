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

// Self-contained execution stack
typedef struct {
    int64_t data[STACK_CAPACITY];
    int top;
} vppd_stack_t;

// vppd.bin main executable context
typedef struct {
    vppd_stack_t stack;
    uint64_t total_packets;
    double pll_phase_error;
    uint64_t tsc_cycles;
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

// CICS ASCII Terminal interface rendering
static void render_cics_terminal(const vppd_context_t *ctx) {
    printf("\n========================================\n");
    printf("--- CICS TERMINAL DIAGNOSTIC SCREEN ---\n");
    printf("  VPPD STATUS    : ACTIVE (Self-Contained)\n");
    printf("  PACKETS SWITCHED: %lu\n", ctx->total_packets);
    printf("  PLL PHASE ERROR: %.4f\n", ctx->pll_phase_error);
    printf("  SYSTEM CYCLES  : %lu\n", ctx->tsc_cycles);
    printf("  STACK DEPTH    : %d\n", ctx->stack.top);
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
            sprintf(out_status, "OK;packets=%lu;phase=%.4f", ctx->total_packets, ctx->pll_phase_error);
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
            printf("  ping       - Query daemon responsiveness\n");
            printf("  status     - Query current telemetry statistics\n");
            printf("  route <N>  - Route a simulated transaction packet with sequence N\n");
            printf("  exit       - Terminate the session\n");
            fflush(stdout);
        } else if (strcmp(input_line, "ping") == 0) {
            vppd_abi_entry(ctx, ABI_CMD_PING, NULL, abi_buffer);
            printf("Result: %s\n", abi_buffer);
            fflush(stdout);
        } else if (strcmp(input_line, "status") == 0) {
            vppd_abi_entry(ctx, ABI_CMD_STATUS, NULL, abi_buffer);
            printf("Result: %s\n", abi_buffer);
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

    printf("=============================================================\n");
    printf("VPPD.BIN SELF-PROFILING TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
