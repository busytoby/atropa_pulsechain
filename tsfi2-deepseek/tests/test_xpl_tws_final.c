#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// 1. Dynamic Library Linker (XPLSM)
typedef struct {
    char func_name[32];
    unsigned int address;
} SharedLibrarySymbol;

static SharedLibrarySymbol symbol_table[16];
static int symbol_count = 0;

void xplsm_register_shared_symbol(const char *name, unsigned int addr) {
    strcpy(symbol_table[symbol_count].func_name, name);
    symbol_table[symbol_count].address = addr;
    symbol_count++;
}

unsigned int xplsm_resolve_symbol(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].func_name, name) == 0) {
            return symbol_table[i].address;
        }
    }
    return 0; // Symbol unresolved
}

// 2. Common Subexpression Elimination (CSE)
typedef struct {
    char expression[64];
    char temp_var[16];
} CseEntry;

void xcom_cse_optimize(const char *input_yul, char *out_yul) {
    out_yul[0] = '\0';
    CseEntry cse_table[16];
    int cse_count = 0;
    
    char src_copy[1024];
    strcpy(src_copy, input_yul);
    
    char *line = strtok(src_copy, "\n");
    while (line != NULL) {
        char optimized_line[256];
        strcpy(optimized_line, line);
        
        // Find expressions like add(a, b)
        char *p_expr = strstr(line, "add(");
        if (p_expr) {
            char expr_buf[64];
            sscanf(p_expr, "%[^;]", expr_buf);
            
            // Check if this subexpression has already been computed
            int found_idx = -1;
            for (int i = 0; i < cse_count; i++) {
                if (strcmp(cse_table[i].expression, expr_buf) == 0) {
                    found_idx = i;
                    break;
                }
            }
            
            if (found_idx != -1) {
                // Eliminate subexpression by reusing the temp variable
                printf("[XCOM] CSE: Reusing previously computed value from %s\n", cse_table[found_idx].temp_var);
                sprintf(optimized_line, "let %.*s := %s", (int)(p_expr - line - 4), line, cse_table[found_idx].temp_var);
            } else {
                // Record new subexpression
                strcpy(cse_table[cse_count].expression, expr_buf);
                sprintf(cse_table[cse_count].temp_var, "t_%d", cse_count);
                printf("[XCOM] CSE: Recording subexpression: %s -> %s\n", expr_buf, cse_table[cse_count].temp_var);
                cse_count++;
            }
        }
        
        strcat(out_yul, optimized_line);
        strcat(out_yul, "\n");
        line = strtok(NULL, "\n");
    }
}

// 3. Control Flow Integrity (CFI) Verification
typedef struct {
    unsigned int address;
    bool is_landing_pad;
} CfiTarget;

bool xcom_verify_cfi_target(unsigned int target_addr, const CfiTarget *targets, int count) {
    for (int i = 0; i < count; i++) {
        if (targets[i].address == target_addr) {
            if (targets[i].is_landing_pad) {
                return true; // Valid jump target
            } else {
                printf("[XCOM] CFI Violation: Attempted indirect jump to non-landing pad offset %u.\n", target_addr);
                return false;
            }
        }
    }
    printf("[XCOM] CFI Violation: Target address %u is not registered.\n", target_addr);
    return false;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== TSFi ZMM VM Auncient XPL TWS Final Upgrades Test ===\n");

    // Test 1: Dynamic Library Linker (XPLSM)
    printf("\n--- Final Case 1: XPLSM Dynamic Linker ---\n");
    xplsm_register_shared_symbol("vceWriteColor", 0xF300);
    xplsm_register_shared_symbol("psgWriteVol", 0xF101);
    
    unsigned int addr1 = xplsm_resolve_symbol("vceWriteColor");
    assert(addr1 == 0xF300);
    unsigned int addr2 = xplsm_resolve_symbol("invalidFunc");
    assert(addr2 == 0);
    printf("[XPLSM] Success: Dynamic library symbols successfully resolved at runtime.\n");

    // Test 2: Common Subexpression Elimination (CSE)
    printf("\n--- Final Case 2: CSE Optimizer ---\n");
    const char *yul_src = "let x := add(a, b)\nlet y := add(a, b)";
    char optimized_yul[1024];
    xcom_cse_optimize(yul_src, optimized_yul);
    printf("[XCOM] CSE Optimized Yul:\n%s", optimized_yul);
    assert(strstr(optimized_yul, "let y := t_0") != NULL);
    printf("[XCOM] Success: Redundant subexpressions eliminated cleanly.\n");

    // Test 3: Control Flow Integrity (CFI) Verification
    printf("\n--- Final Case 3: CFI Jump Verification ---\n");
    CfiTarget targets[3] = {
        {0x1000, true},  // Valid function entry (landing pad)
        {0x1004, false}, // Mid-instruction (invalid!)
        {0x2000, true}   // Valid function entry
    };
    
    assert(xcom_verify_cfi_target(0x1000, targets, 3) == true);
    assert(xcom_verify_cfi_target(0x1004, targets, 3) == false);
    assert(xcom_verify_cfi_target(0x9999, targets, 3) == false);
    printf("[XCOM] Success: CFI checks successfully blocked control-hijack branches.\n");

    printf("\n=== ALL FINAL TWS UPGRADE TESTS PASSED ===\n");
    return 0;
}
