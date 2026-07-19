#include "tsfi_zorse_eval.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("[Zorse Eval Test] Starting JCL and COBOL compliance audits...\n");

    // Test Case 1: Valid JCL JOB statement
    const char *valid_jcl = "//MYJOB JOB (123),'TEST',CLASS=A\n//STEP1 EXEC PGM=IEFBR14\n";
    int is_valid = 0;
    char error_msg[256];
    assert(tsfi_zorse_validate_jcl(valid_jcl, &is_valid, error_msg, sizeof(error_msg)) == 0);
    assert(is_valid == 1);

    // Test Case 2: Invalid JCL (missing prefix)
    const char *invalid_jcl_prefix = "MYJOB JOB (123),'TEST',CLASS=A\n";
    assert(tsfi_zorse_validate_jcl(invalid_jcl_prefix, &is_valid, error_msg, sizeof(error_msg)) == 0);
    assert(is_valid == 0);
    assert(strstr(error_msg, "Invalid prefix") != NULL);

    // Test Case 3: Invalid JCL (missing JOB card)
    const char *invalid_jcl_job = "//MYJOB CLASS=A\n";
    assert(tsfi_zorse_validate_jcl(invalid_jcl_job, &is_valid, error_msg, sizeof(error_msg)) == 0);
    assert(is_valid == 0);
    assert(strstr(error_msg, "Missing mandatory JCL JOB statement") != NULL);

    // Test Case 4: Valid COBOL division sequences
    const char *valid_cobol = "IDENTIFICATION DIVISION.\nPROGRAM-ID. HELLO.\nPROCEDURE DIVISION.\nDISPLAY 'HELLO'.\n";
    assert(tsfi_zorse_validate_cobol(valid_cobol, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 5: Invalid COBOL division sequences (reversed)
    const char *invalid_cobol = "PROCEDURE DIVISION.\nDISPLAY 'HELLO'.\nIDENTIFICATION DIVISION.\nPROGRAM-ID. HELLO.\n";
    assert(tsfi_zorse_validate_cobol(invalid_cobol, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 6: Valid JCL DD Statement
    const char *valid_dd = "//SYSUT1   DD DSN=MY.DATA.SET,DISP=SHR\n";
    assert(tsfi_zorse_validate_jcl_dd(valid_dd, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 7: Invalid JCL DD Statement (missing DISP)
    const char *invalid_dd = "//SYSUT1   DD DSN=MY.DATA.SET\n";
    assert(tsfi_zorse_validate_jcl_dd(invalid_dd, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 8: Valid HLASM Instruction
    const char *valid_hlasm = "         LA    R1,MYADDR";
    assert(tsfi_zorse_validate_hlasm(valid_hlasm, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 9: Invalid HLASM Instruction (spaces in operands)
    const char *invalid_hlasm = "         LA    R1, MYADDR";
    assert(tsfi_zorse_validate_hlasm(invalid_hlasm, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 10: LLM query interface validation
    char llm_res[256];
    int q_res = tsfi_zorse_query_llm("Ping", "moondream", llm_res, sizeof(llm_res));
    // If the Ollama server is offline, it should gracefully return -2 (connect failure) or -1 (bad args)
    assert(q_res == 0 || q_res == -2);

    printf("[PASS] Zorse compliance evaluation tests verified successfully!\n");
    return 0;
}
