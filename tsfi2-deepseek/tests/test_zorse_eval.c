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

    // Test Case 11: Valid COBOL PICTURE Clause
    const char *valid_pic = "01  MY-VAR  PIC X(10).\n";
    assert(tsfi_zorse_validate_cobol_pic(valid_pic, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 12: Invalid COBOL PICTURE Clause (missing format)
    const char *invalid_pic = "01  MY-VAR  PIC.\n";
    assert(tsfi_zorse_validate_cobol_pic(invalid_pic, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 13: Valid JCL EXEC Program
    const char *valid_pgm = "//STEP1 EXEC PGM=IEFBR14,PARM='TEST'\n";
    assert(tsfi_zorse_validate_jcl_pgm(valid_pgm, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 14: Invalid JCL EXEC Program (too long)
    const char *invalid_pgm = "//STEP1 EXEC PGM=IEFBR14LONG\n";
    assert(tsfi_zorse_validate_jcl_pgm(invalid_pgm, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 15: Valid COBOL REDEFINES Clause
    const char *valid_redef = "05  VAR2  REDEFINES VAR1.\n";
    assert(tsfi_zorse_validate_cobol_redefines(valid_redef, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 16: Invalid COBOL REDEFINES Clause (missing period)
    const char *invalid_redef = "05  VAR2  REDEFINES VAR1\n";
    assert(tsfi_zorse_validate_cobol_redefines(invalid_redef, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 17: Valid JCL Symbolic Parameter
    const char *valid_sym = "//STEP1 EXEC PGM=MYPROG,PARM='&MYVAR'\n";
    assert(tsfi_zorse_validate_jcl_symbolic(valid_sym, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 18: Invalid JCL Symbolic Parameter (numeric start)
    const char *invalid_sym = "//STEP1 EXEC PGM=MYPROG,PARM='&1MYVAR'\n";
    assert(tsfi_zorse_validate_jcl_symbolic(invalid_sym, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 19: Valid COBOL OCCURS Clause
    const char *valid_occurs = "05  MY-ARRAY  PIC X OCCURS 10 TIMES.\n";
    assert(tsfi_zorse_validate_cobol_occurs(valid_occurs, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 20: Invalid COBOL OCCURS Clause (exceeds bounds limit)
    const char *invalid_occurs = "05  MY-ARRAY  PIC X OCCURS 99999 TIMES.\n";
    assert(tsfi_zorse_validate_cobol_occurs(invalid_occurs, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 21: Valid JCL COND Parameter
    const char *valid_cond = "//STEP1 EXEC PGM=MYPROG,COND=(4,LT)\n";
    assert(tsfi_zorse_validate_jcl_cond(valid_cond, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 22: Invalid JCL COND Parameter (invalid operator)
    const char *invalid_cond = "//STEP1 EXEC PGM=MYPROG,COND=(4,XX)\n";
    assert(tsfi_zorse_validate_jcl_cond(invalid_cond, &is_valid) == 0);
    assert(is_valid == 0);

    // Test Case 23: LLM Autocorrect Pipeline validation
    char corrected_buf[256];
    int autocor_res = tsfi_zorse_autocorrect_source("10 DISPLAY", "COBOL", "moondream", corrected_buf, sizeof(corrected_buf));
    assert(autocor_res == 0 || autocor_res == -2);

    // Test Case 24: LLM Dependency Resolver validation
    char dep_buf[256];
    int dep_res = tsfi_zorse_resolve_dependencies("SELECT F1 ASSIGN TO UT-S-FILE1.", "//FILE1 DD DSN=A.B.C", "moondream", dep_buf, sizeof(dep_buf));
    assert(dep_res == 0 || dep_res == -2);

    // Test Case 25: Moondream Visual Console Auditor validation
    char visual_res[256];
    int vis_res = tsfi_zorse_audit_screen_visual("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", visual_res, sizeof(visual_res));
    assert(vis_res == 0 || vis_res == -2 || vis_res == -1);

    // Test Case 26: LLM COBOL-to-HLASM Transpiler validation
    char transp_res[256];
    int transp_res_val = tsfi_zorse_transpile_cobol_to_hlasm("DISPLAY 'HELLO'.", "moondream", transp_res, sizeof(transp_res));
    assert(transp_res_val == 0 || transp_res_val == -2);

    // Test Case 27: Moondream Flowchart-to-JCL Generator validation
    char flowchart_res[256];
    int flow_res = tsfi_zorse_generate_jcl_from_flowchart("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", flowchart_res, sizeof(flowchart_res));
    assert(flow_res == 0 || flow_res == -2 || flow_res == -1);

    // Test Case 28: LLM HLASM-to-COBOL Reverse Transpiler validation
    char cobol_transp_res[256];
    int reverse_res = tsfi_zorse_transpile_hlasm_to_cobol("BALR 12,0", "moondream", cobol_transp_res, sizeof(cobol_transp_res));
    assert(reverse_res == 0 || reverse_res == -2);

    printf("[PASS] Zorse compliance evaluation tests verified successfully!\n");
    return 0;
}
