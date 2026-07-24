#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SEGMENT_SIZE 64

// Simulated VDM memory layout
typedef struct {
    char segment_a[SEGMENT_SIZE];
    char segment_b[SEGMENT_SIZE];
    uint8_t mutex_reg; // Hardware lock register
} xplos_vdm_mem_t;

// XplOS Task state
typedef struct {
    uint32_t task_id;
    uint32_t target_address; // Horning overlap check target
    bool has_error;
} xplos_task_ctx_t;

// XplOS Binary Header: Converse Vector Table (CVT)
typedef struct {
    uint32_t task_count;
    void (*converse_handlers[2])(xplos_vdm_mem_t *mem);
} xplos_binary_header_t;

// Global state backups for converse recovery
static char backup_a[SEGMENT_SIZE];
static char backup_b[SEGMENT_SIZE];

// -------------------------------------------------------------
// Converse Handlers (CVT targets)
// -------------------------------------------------------------
void handler_converse_task1(xplos_vdm_mem_t *mem) {
    strcpy(mem->segment_a, backup_a);
    mem->mutex_reg = 0; // Release lock
}

void handler_converse_task2(xplos_vdm_mem_t *mem) {
    strcpy(mem->segment_b, backup_b);
    mem->mutex_reg = 0; // Release lock
}

// -------------------------------------------------------------
// Parallel Execution Simulator (APDL)
// -------------------------------------------------------------
bool execute_xplos_binary(const xplos_binary_header_t *header,
                           xplos_task_ctx_t *tasks,
                           xplos_vdm_mem_t *mem,
                           const char *val_a,
                           const char *val_b) {
    // 1. Horning Overlap Audit at binary initialization
    if (tasks[0].target_address == tasks[1].target_address) {
        return false; // Collision: tasks overlap target addresses
    }

    // Back up current memory states for converse recovery
    strcpy(backup_a, mem->segment_a);
    strcpy(backup_b, mem->segment_b);

    // 2. Parallel Task 1 (alpha) Execution
    if (mem->mutex_reg == 0) {
        mem->mutex_reg = 1; // Acquire lock
        if (tasks[0].has_error) {
            header->converse_handlers[0](mem); // Trigger CVT rollback
            return false;
        }
        strcpy(mem->segment_a, val_a);
        mem->mutex_reg = 0; // Release lock
    }

    // 3. Parallel Task 2 (beta) Execution
    if (mem->mutex_reg == 0) {
        mem->mutex_reg = 2; // Acquire lock
        if (tasks[1].has_error) {
            header->converse_handlers[1](mem); // Trigger CVT rollback
            return false;
        }
        strcpy(mem->segment_b, val_b);
        mem->mutex_reg = 0; // Release lock
    }

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS BINARY APDL VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    xplos_vdm_mem_t mem = {
        .segment_a = "INIT_A",
        .segment_b = "INIT_B",
        .mutex_reg = 0
    };

    xplos_binary_header_t header = {
        .task_count = 2,
        .converse_handlers = { handler_converse_task1, handler_converse_task2 }
    };

    // 1. Compliant Parallel Execution -> Should succeed
    xplos_task_ctx_t compliant_tasks[2] = {
        { .task_id = 1, .target_address = 0xF000, .has_error = false },
        { .task_id = 2, .target_address = 0xF100, .has_error = false }
    };

    printf("[TEST] Dispatching compliant parallel XplOS tasks...\n");
    fflush(stdout);
    bool ok = execute_xplos_binary(&header, compliant_tasks, &mem, "COMMIT_A", "COMMIT_B");
    assert(ok == true);
    assert(strcmp(mem.segment_a, "COMMIT_A") == 0);
    assert(strcmp(mem.segment_b, "COMMIT_B") == 0);
    assert(mem.mutex_reg == 0);
    printf("   ✓ Both segments updated. Mutex released.\n");
    fflush(stdout);

    // 2. Horning Collision Block -> Should refuse execution
    xplos_task_ctx_t colliding_tasks[2] = {
        { .task_id = 1, .target_address = 0xF000, .has_error = false },
        { .task_id = 2, .target_address = 0xF000, .has_error = false } // Same address
    };

    printf("[TEST] Dispatching colliding parallel tasks (Horning Check)...\n");
    fflush(stdout);
    ok = execute_xplos_binary(&header, colliding_tasks, &mem, "NEW_A", "NEW_B");
    assert(ok == false);
    printf("   ✓ Write segment overlap blocked successfully.\n");
    fflush(stdout);

    // 3. Task failure -> Should trigger CVT converse handler rollback
    xplos_task_ctx_t failing_tasks[2] = {
        { .task_id = 1, .target_address = 0xF000, .has_error = false },
        { .task_id = 2, .target_address = 0xF100, .has_error = true } // Injects error
    };

    printf("[TEST] Dispatching parallel tasks with failure injection...\n");
    fflush(stdout);
    ok = execute_xplos_binary(&header, failing_tasks, &mem, "NEW_A", "NEW_B");
    assert(ok == false);
    // Segment B should be restored to backup (which was "COMMIT_B" after the first run)
    assert(strcmp(mem.segment_b, "COMMIT_B") == 0);
    assert(mem.mutex_reg == 0);
    printf("   ✓ Failure trapped. Converse handler rolled back Segment B.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("XPLOS BINARY APDL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
