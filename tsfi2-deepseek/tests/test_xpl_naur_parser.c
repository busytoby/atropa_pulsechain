#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>

// Naur-Form Grammar rule verification structure
typedef struct {
    char opcode_str[32];
    uint32_t payload_val;
    bool has_approvals;
    int approvals[4];
} parsed_instruction_t;

// BNF Syntax Checker: Parses <instruction> ::= <opcode> <space> <value> [ <space> <approvals> ]
static bool parse_naur_line(const char *line, parsed_instruction_t *out) {
    if (!line || !out) return false;
    memset(out, 0, sizeof(parsed_instruction_t));

    char op[64] = {0};
    uint32_t val = 0;
    int a0 = 0, a1 = 0, a2 = 0, a3 = 0;

    // Scan for five-parameter structure: WRITE_ABD <val> <a0> <a1> <a2> <a3>
    int scanned = sscanf(line, "%63s %u %d %d %d %d", op, &val, &a0, &a1, &a2, &a3);
    if (scanned == 6) {
        if (strcmp(op, "WRITE_ABD") != 0) return false;
        snprintf(out->opcode_str, sizeof(out->opcode_str), "%s", op);
        out->payload_val = val;
        out->has_approvals = true;
        out->approvals[0] = a0;
        out->approvals[1] = a1;
        out->approvals[2] = a2;
        out->approvals[3] = a3;
        return true;
    }

    // Scan for single-parameter structure: LOAD_SUB_XPL <val> or INIT_RAU <val>
    scanned = sscanf(line, "%63s %u", op, &val);
    if (scanned == 2) {
        if (strcmp(op, "LOAD_SUB_XPL") != 0 && strcmp(op, "INIT_RAU") != 0) return false;
        snprintf(out->opcode_str, sizeof(out->opcode_str), "%s", op);
        out->payload_val = val;
        out->has_approvals = false;
        return true;
    }

    // Scan for zero-parameter structure: READ_KERMIT
    scanned = sscanf(line, "%63s", op);
    if (scanned == 1) {
        if (strcmp(op, "READ_KERMIT") != 0) return false;
        snprintf(out->opcode_str, sizeof(out->opcode_str), "%s", op);
        out->payload_val = 0;
        out->has_approvals = false;
        return true;
    }

    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPL COMPILER NAUR-FORM GRAMMAR VALIDATION SUITE\n");
    printf("=============================================================\n");

    // 1. Syntax Rules Unit Verification
    printf("[TEST] Verifying compliant BNF grammar statements...\n");
    parsed_instruction_t instr;

    assert(parse_naur_line("WRITE_ABD 1234 1 0 1 0", &instr) == true);
    assert(strcmp(instr.opcode_str, "WRITE_ABD") == 0);
    assert(instr.payload_val == 1234);
    assert(instr.approvals[0] == 1 && instr.approvals[2] == 1);

    assert(parse_naur_line("INIT_RAU 42", &instr) == true);
    assert(strcmp(instr.opcode_str, "INIT_RAU") == 0);
    assert(instr.payload_val == 42);

    assert(parse_naur_line("READ_KERMIT", &instr) == true);
    assert(strcmp(instr.opcode_str, "READ_KERMIT") == 0);

    printf("   ✓ Compliant Naur-form lines successfully validated.\n");

    printf("[TEST] Testing malformed syntax rejection...\n");
    assert(parse_naur_line("INVALID_OP 100", &instr) == false);
    assert(parse_naur_line("WRITE_ABD abc", &instr) == false);
    assert(parse_naur_line("", &instr) == false);
    printf("   ✓ Malformed input streams successfully rejected.\n");

    // 2. Parser Performance Benchmarking
    printf("[BENCHMARK] Running parser execution loop (100,000 iterations)...\n");
    const char *bench_line = "WRITE_ABD 999999 1 1 1 1";
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 100000; i++) {
        bool ok = parse_naur_line(bench_line, &instr);
        (void)ok;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    double latency = elapsed_ns / 100000.0;

    printf("   ✓ Parsed 100,000 instructions in %.2f ms (Average: %.2f ns/op).\n", elapsed_ns / 1e6, latency);
    assert(latency < 1000.0); // Sub-microsecond latency limit check

    printf("=============================================================\n");
    printf("ALL NAUR-FORM COMPILER TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
