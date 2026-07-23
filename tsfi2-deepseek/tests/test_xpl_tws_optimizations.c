#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define REG_VCE_BASE 62208

// 1. Constant Folding & Dead Code Elimination Mock
int xcom_optimize_expression(const char *expr_str, int *out_val) {
    // Basic parser for "A + B" constant folding
    int val_a = 0, val_b = 0;
    if (sscanf(expr_str, "%d + %d", &val_a, &val_b) == 2) {
        *out_val = val_a + val_b;
        return 0; // Folded successfully
    }
    return -1;
}

int xcom_prune_dead_code(const char *condition_str, const char *code_block, char *out_yul) {
    // If condition is constant 0, prune block (Dead Code Elimination)
    if (strcmp(condition_str, "0") == 0) {
        out_yul[0] = '\0'; // Emit nothing
        return 1; // Pruned
    }
    // Otherwise emit code block
    strcpy(out_yul, code_block);
    return 0; // Kept
}

// 2. Binary Precedence Table Serialization (.dat.bin)
typedef struct {
    uint8_t precedence_matrix[8][8];
    uint32_t magic_header;
} PrecedenceTableFile;

int analyzer_serialize_table(const char *file_path, const PrecedenceTableFile *table) {
    // Rule 13 check: must use .dat.bin extension
    if (!strstr(file_path, ".dat.bin")) {
        return -1;
    }
    FILE *f = fopen(file_path, "wb");
    if (!f) return -2;
    fwrite(table, sizeof(PrecedenceTableFile), 1, f);
    fclose(f);
    return 0;
}

int analyzer_deserialize_table(const char *file_path, PrecedenceTableFile *out_table) {
    if (!strstr(file_path, ".dat.bin")) {
        return -1;
    }
    FILE *f = fopen(file_path, "rb");
    if (!f) return -2;
    size_t nread = fread(out_table, sizeof(PrecedenceTableFile), 1, f);
    fclose(f);
    return (nread == 1) ? 0 : -3;
}

// 3. Speculative Instruction Scheduling
typedef struct {
    char instruction[128];
    bool is_store;
} YulInstruction;

void skeleton_schedule_instructions(YulInstruction *list, int count, char *out_yul) {
    out_yul[0] = '\0';
    
    // Group store instructions together (speculative write scheduling)
    printf("[SKELETON] Speculatively scheduling instruction pipeline...\n");
    
    // Pass 1: Emit all stores
    for (int i = 0; i < count; i++) {
        if (list[i].is_store) {
            strcat(out_yul, list[i].instruction);
            strcat(out_yul, "\n");
        }
    }
    // Pass 2: Emit other instructions
    for (int i = 0; i < count; i++) {
        if (!list[i].is_store) {
            strcat(out_yul, list[i].instruction);
            strcat(out_yul, "\n");
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== TSFi ZMM VM Auncient XPL TWS Optimizations Test ===\n");

    // Test 1: Constant Folding & Dead Code Elimination
    printf("\n--- Optimization Case 1: XCOM Constant Fold & Dead Code ---\n");
    int folded_val = 0;
    int fold_res = xcom_optimize_expression("150 + 156", &folded_val);
    assert(fold_res == 0);
    assert(folded_val == 306);
    printf("[XCOM] Constant Folding: '150 + 156' successfully folded to %u\n", folded_val);

    char emitted_yul[256];
    int prune_res = xcom_prune_dead_code("0", "storeNamespaced(62208, 99)", emitted_yul);
    assert(prune_res == 1);
    assert(strlen(emitted_yul) == 0);
    printf("[XCOM] Dead Code Elimination: Block containing 'IF (0)' pruned successfully.\n");

    // Test 2: Binary Precedence Table Serialization (.dat.bin)
    printf("\n--- Optimization Case 2: ANALYZER Table Serialization (.dat.bin) ---\n");
    PrecedenceTableFile mock_table;
    mock_table.magic_header = 0xAA5500E0;
    memset(mock_table.precedence_matrix, 2, sizeof(mock_table.precedence_matrix));
    
    const char *table_file = "precedence_table.dat.bin"; // Rule 13 compliant
    int ser_res = analyzer_serialize_table(table_file, &mock_table);
    assert(ser_res == 0);

    PrecedenceTableFile loaded_table;
    int deser_res = analyzer_deserialize_table(table_file, &loaded_table);
    assert(deser_res == 0);
    assert(loaded_table.magic_header == 0xAA5500E0);
    assert(loaded_table.precedence_matrix[0][0] == 2);
    printf("[ANALYZER] Success: Precedence table serialized/deserialized via %s.\n", table_file);
    remove(table_file);

    // Test 3: Speculative Instruction Scheduling
    printf("\n--- Optimization Case 3: SKELETON Speculative Scheduling ---\n");
    YulInstruction list[3] = {
        {"let val_x := 10", false},
        {"storeNamespaced(62208, val_x)", true},
        {"let val_y := 20", false}
    };
    char scheduled_yul[1024];
    skeleton_schedule_instructions(list, 3, scheduled_yul);
    printf("[SKELETON] Scheduled Yul Output:\n%s", scheduled_yul);
    
    // Verify store is scheduled first
    assert(strncmp(scheduled_yul, "storeNamespaced", 15) == 0);
    printf("[SKELETON] Success: Stores prioritized to optimize VM pre-fetch pipeline.\n");

    printf("\n=== ALL TWS OPTIMIZATION TESTS PASSED ===\n");
    return 0;
}
