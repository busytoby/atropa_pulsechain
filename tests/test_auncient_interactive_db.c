#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Simulated VDM State for XPL debug target
typedef struct {
    uint8_t mutex_reg;
    uint8_t coax_token_reg;
    uint32_t register_space[8];
    uint32_t program_counter;
    bool halted;
} debug_vdm_state_t;

// Simulated instruction format
typedef struct {
    char label[32];
    char op[16];
    uint32_t reg_idx;
    uint32_t val;
} xpl_instruction_t;

// -------------------------------------------------------------
// Interactive Debugger Command Loop
// -------------------------------------------------------------
void print_debugger_help(void) {
    printf("XPL Interactive Debugger Command Options:\n");
    printf("  step / s      Execute the current instruction\n");
    printf("  show / p      Print current register state and memory status\n");
    printf("  help / h      Display command documentation\n");
    printf("  quit / q      Terminate debug session\n");
}

void print_vdm_state(const debug_vdm_state_t *state) {
    printf("--- VDM State ---\n");
    printf("Program Counter: %u\n", state->program_counter);
    printf("Mutex Registry : %u\n", state->mutex_reg);
    printf("Coax Token     : %u\n", state->coax_token_reg);
    for (int i = 0; i < 8; i++) {
        printf("Register [%d]  : %u\n", i, state->register_space[i]);
    }
    printf("Status         : %s\n", state->halted ? "HALTED" : "RUNNING");
    printf("-----------------\n");
}

int main(int argc, char **argv) {
    printf("=============================================================\n");
    printf("AUNCIENT INTERACTIVE XPL BINARY DEBUGGER FRONT END\n");
    printf("=============================================================\n");

    // Initialize debugger target
    debug_vdm_state_t state = {
        .mutex_reg = 0,
        .coax_token_reg = 0,
        .register_space = { 0 },
        .program_counter = 0,
        .halted = false
    };

    // Load simple simulated instruction array (mimics compiled APDL xpl output)
    xpl_instruction_t program[4] = {
        { .label = "ACQUIRE_LOCK", .op = "SET_MUTEX", .reg_idx = 0, .val = 1 },
        { .label = "WRITE_VALUE",  .op = "WRITE_REG", .reg_idx = 4, .val = 99 },
        { .label = "RELEASE_LOCK", .op = "SET_MUTEX", .reg_idx = 0, .val = 0 },
        { .label = "HALT_PROGRAM", .op = "HALT",      .reg_idx = 0, .val = 0 }
    };

    bool interactive = (argc < 2 || strcmp(argv[1], "--non-interactive") != 0);

    char cmd[128];
    print_debugger_help();

    while (!state.halted) {
        if (state.program_counter >= 4) {
            state.halted = true;
            break;
        }

        xpl_instruction_t current = program[state.program_counter];
        printf("\nNext [PC %u]: %s %s(Reg %u, Val %u)\n", 
               state.program_counter, current.label, current.op, current.reg_idx, current.val);

        if (interactive) {
            printf("xpl-db> ");
            fflush(stdout);
            if (!fgets(cmd, sizeof(cmd), stdin)) {
                break;
            }
            // Strip newline
            cmd[strcspn(cmd, "\r\n")] = 0;
        } else {
            // Non-interactive auto-step mode for validation sweeps
            printf("Auto-stepping [PC %u]...\n", state.program_counter);
            strcpy(cmd, "step");
        }

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            printf("Terminating debugger session.\n");
            break;
        }
        else if (strcmp(cmd, "show") == 0 || strcmp(cmd, "p") == 0) {
            print_vdm_state(&state);
        }
        else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
            print_debugger_help();
        }
        else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
            // Execute instruction step
            if (strcmp(current.op, "SET_MUTEX") == 0) {
                state.mutex_reg = current.val;
            }
            else if (strcmp(current.op, "WRITE_REG") == 0) {
                state.register_space[current.reg_idx] = current.val;
            }
            else if (strcmp(current.op, "HALT") == 0) {
                state.halted = true;
            }
            state.program_counter++;
            printf("   Instruction executed successfully.\n");
        }
        else {
            printf("Error: Unknown command. Enter 'help' for options.\n");
        }
    }

    printf("\nDebugger session closed. Final state:\n");
    print_vdm_state(&state);
    printf("=============================================================\n");
    return 0;
}
