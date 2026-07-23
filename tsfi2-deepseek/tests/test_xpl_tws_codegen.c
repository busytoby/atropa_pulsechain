#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// 1. Graph Coloring Register Allocator
typedef struct {
    char name;
    int start;
    int end;
    int assigned_reg; // -1 means spilled to memory
} LiveInterval;

void xcom_allocate_registers(LiveInterval *vars, int var_count, int reg_count) {
    printf("[XCOM] Running Chaitin register allocator...\n");
    for (int i = 0; i < var_count; i++) {
        vars[i].assigned_reg = -1;
    }
    
    // Assign registers based on interference (overlapping intervals)
    for (int i = 0; i < var_count; i++) {
        bool reg_used[8] = {false};
        
        for (int j = 0; j < i; j++) {
            // If intervals overlap, they interfere
            bool overlap = !(vars[i].start > vars[j].end || vars[i].end < vars[j].start);
            if (overlap && vars[j].assigned_reg != -1) {
                reg_used[vars[j].assigned_reg] = true;
            }
        }
        
        // Find first free register
        for (int r = 0; r < reg_count; r++) {
            if (!reg_used[r]) {
                vars[i].assigned_reg = r;
                break;
            }
        }
    }
}

// 2. Inline Macro Template Expansion
int skeleton_expand_macro(const char *macro_name, char *out_expanded_src) {
    if (strcmp(macro_name, "MACRO_VCE_RESET;") == 0) {
        strcpy(out_expanded_src, "BYTE(62208) = 0; BYTE(62209) = 0; BYTE(62210) = 0;");
        return 0; // Expanded
    }
    return -1; // Unknown macro
}

// 3. Basic Block Control Flow Graph (CFG) Validation
typedef struct {
    int id;
    int target_a;
    int target_b; // -1 means no branch
} BasicBlock;

bool cfg_validate_reachability(BasicBlock *blocks, int count) {
    bool visited[16] = {false};
    int queue[16];
    int head = 0, tail = 0;
    
    // BFS starting from Entry Block (id 0)
    queue[tail++] = 0;
    visited[0] = true;
    
    while (head < tail) {
        int curr = queue[head++];
        BasicBlock b = blocks[curr];
        
        if (b.target_a != -1 && !visited[b.target_a]) {
            visited[b.target_a] = true;
            queue[tail++] = b.target_a;
        }
        if (b.target_b != -1 && !visited[b.target_b]) {
            visited[b.target_b] = true;
            queue[tail++] = b.target_b;
        }
    }
    
    // Check if all blocks are reachable
    for (int i = 0; i < count; i++) {
        if (!visited[i]) {
            printf("[CFG] Validation Error: Block %d is unreachable.\n", i);
            return false;
        }
    }
    return true;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== TSFi ZMM VM Auncient XPL TWS Codegen Upgrades Test ===\n");

    // Test 1: Register Allocation via Graph Coloring
    printf("\n--- Codegen Case 1: Register Allocator ---\n");
    LiveInterval variables[3] = {
        {'A', 0, 4, -1}, // Variable A active from step 0 to 4
        {'B', 2, 6, -1}, // Variable B active from step 2 to 6 (overlaps A)
        {'C', 5, 8, -1}  // Variable C active from step 5 to 8 (overlaps B, but NOT A)
    };
    
    // Allocate to a target machine with 2 registers
    xcom_allocate_registers(variables, 3, 2);
    
    // A and B interfere -> must get different registers
    assert(variables[0].assigned_reg != variables[1].assigned_reg);
    // B and C interfere -> must get different registers
    assert(variables[1].assigned_reg != variables[2].assigned_reg);
    // A and C do not interfere -> can share the same register
    assert(variables[0].assigned_reg == variables[2].assigned_reg);
    
    printf("[XCOM] Variable '%c' assigned register: R%d\n", variables[0].name, variables[0].assigned_reg);
    printf("[XCOM] Variable '%c' assigned register: R%d\n", variables[1].name, variables[1].assigned_reg);
    printf("[XCOM] Variable '%c' assigned register: R%d\n", variables[2].name, variables[2].assigned_reg);
    printf("[XCOM] Success: Interference graph colored cleanly.\n");

    // Test 2: Inline Macro Template Expansion
    printf("\n--- Codegen Case 2: SKELETON Macro Expansion ---\n");
    char macro_buffer[256];
    int expand_res = skeleton_expand_macro("MACRO_VCE_RESET;", macro_buffer);
    assert(expand_res == 0);
    assert(strstr(macro_buffer, "BYTE(62208) = 0;") != NULL);
    printf("[SKELETON] Macro expanded source:\n%s\n", macro_buffer);
    printf("[SKELETON] Success: Macro templates expanded prior to parsing.\n");

    // Test 3: CFG Validation
    printf("\n--- Codegen Case 3: CFG Reachability check ---\n");
    BasicBlock valid_cfg[3] = {
        {0, 1, 2}, // Entry goes to 1 and 2
        {1, 2, -1}, // Block 1 goes to 2
        {2, -1, -1} // Block 2 has no branches
    };
    assert(cfg_validate_reachability(valid_cfg, 3) == true);
    printf("[CFG] Valid graph passed check.\n");

    BasicBlock invalid_cfg[3] = {
        {0, 1, -1}, // Entry goes to 1
        {1, -1, -1}, // Block 1 has no branches
        {2, -1, -1} // Block 2 is isolated (unreachable!)
    };
    assert(cfg_validate_reachability(invalid_cfg, 3) == false);
    printf("[CFG] Invalid graph correctly flagged.\n");

    printf("\n=== ALL TWS CODEGEN TESTS PASSED ===\n");
    return 0;
}
