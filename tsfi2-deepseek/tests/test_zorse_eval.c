#include "tsfi_zorse_eval.h"
#include "tsfi_svdag.h"
#include "tsfi_zmm_vm.h"
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

    // Test Case 29: Moondream DASD Layout Space Mapper validation
    char space_res[256];
    int space_res_val = tsfi_zorse_map_dasd_space("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", space_res, sizeof(space_res));
    assert(space_res_val == 0 || space_res_val == -2 || space_res_val == -1);

    // Test Case 30: LLM Code Explain Pipeline validation
    char explanation_buf[512];
    int explain_res = tsfi_zorse_explain_source("MOVE A TO B.", "COBOL", "moondream", explanation_buf, sizeof(explanation_buf));
    assert(explain_res == 0 || explain_res == -2);

    // Test Case 31: Moondream Visual Punch Card Reader validation
    char punch_card_res[256];
    int pc_res = tsfi_zorse_read_punch_card("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", punch_card_res, sizeof(punch_card_res));
    assert(pc_res == 0 || pc_res == -2 || pc_res == -1);

    // Test Case 32: Moondream Tape Mount Auditor validation
    int is_mounted = 0;
    int tape_res = tsfi_zorse_audit_tape_mount("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "TAPE01", "moondream", &is_mounted);
    assert(tape_res == 0 || tape_res == -2 || tape_res == -1);

    // Test Case 33: Moondream Visual Cabling Topology Parser validation
    char topology_res[256];
    int top_res = tsfi_zorse_parse_cabling_topology("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", topology_res, sizeof(topology_res));
    assert(top_res == 0 || top_res == -2 || top_res == -1);

    // Test Case 34: Moondream Cabinet Thermal Graph Auditor validation
    char thermal_res[256];
    int therm_res = tsfi_zorse_audit_thermal_graph("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", thermal_res, sizeof(thermal_res));
    assert(therm_res == 0 || therm_res == -2 || therm_res == -1);

    // Test Case 35: Job Stream Orchestrator validation
    const char *jcl_inp = "//MYJOB JOB (123),CLASS=A\n";
    const char *cobol_inp = "IDENTIFICATION DIVISION.\nPROGRAM-ID. HELLO.\nPROCEDURE DIVISION.\n";
    char report_buf[1024];
    int is_stream_valid = 0;
    int stream_res = tsfi_zorse_audit_job_stream(jcl_inp, cobol_inp, "moondream", &is_stream_valid, report_buf, sizeof(report_buf));
    assert(stream_res == 0);
    assert(is_stream_valid == 1);
    assert(strstr(report_buf, "APPROVED") != NULL);

    // Test Case 36: COBOL-to-C Struct Transpiler validation
    char c_struct_buf[512];
    int c_trans_res = tsfi_zorse_transpile_cobol_to_c("01 MY-VAR PIC X(10).", "moondream", c_struct_buf, sizeof(c_struct_buf));
    assert(c_trans_res == 0 || c_trans_res == -2);

    // Test Case 37: SNA Session Protocol Auditor validation
    int is_sna_valid = 0;
    int sna_res = tsfi_zorse_audit_sna_session("31010303", &is_sna_valid);
    assert(sna_res == 0);
    assert(is_sna_valid == 1);

    // Test Case 38: Moondream Cabinet Fan Acoustic Spectrogram Auditor validation
    char audit_buf[256];
    int spectrogram_res = tsfi_zorse_audit_fan_spectrogram("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", audit_buf, sizeof(audit_buf));
    assert(spectrogram_res == 0 || spectrogram_res == -2 || spectrogram_res == -1);

    // Test Case 39: COBOL-to-JCL SPACE Optimizer validation
    char space_opt_buf[512];
    int space_opt_res = tsfi_zorse_optimize_jcl_space("01 MY-VAR PIC X(1000).", "moondream", space_opt_buf, sizeof(space_opt_buf));
    assert(space_opt_res == 0 || space_opt_res == -2);

    // Test Case 40: COBOL Copybook Resolver validation
    char copybook_buf[256];
    int copy_res = tsfi_zorse_resolve_copybooks("01 MY-VAR. COPY MYCOPY.", copybook_buf, sizeof(copybook_buf));
    assert(copy_res == 0);
    assert(strcmp(copybook_buf, "MYCOPY") == 0);

    // Test Case 41: JCL DD DISP Parameter Validator validation
    const char *disp_inp = "//DD1 DD DSN=A.B.C,DISP=(NEW,CATLG,DELETE)\n";
    assert(tsfi_zorse_validate_jcl_disp(disp_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 42: HLASM Macro Definition Parser validation
    const char *macro_inp = "         MACRO\nMYMAC    DS    0H\n         MEND\n";
    assert(tsfi_zorse_validate_hlasm_macro(macro_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 43: JCL Step Flow Visualizer Prompt Generator validation
    char flow_prompt_buf[512];
    int flow_prompt_res = tsfi_zorse_generate_flow_prompt("//STEP1 EXEC PGM=IEFBR14", "moondream", flow_prompt_buf, sizeof(flow_prompt_buf));
    assert(flow_prompt_res == 0 || flow_prompt_res == -2);

    // Test Case 44: COBOL FILE STATUS Logic Auditor validation
    const char *cobol_status_inp = "SELECT MYFILE ASSIGN TO UT-S-FILE1 FILE STATUS IS MY-STATUS.";
    assert(tsfi_zorse_validate_cobol_file_status(cobol_status_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 45: JCL SYSOUT Class Auditor validation
    const char *sysout_line = "//SYSOUT DD SYSOUT=*\n";
    assert(tsfi_zorse_validate_jcl_sysout(sysout_line, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 46: HLASM Register Usage Tracker validation
    const char *register_line = "         USING MYBASE,R12\n";
    assert(tsfi_zorse_validate_hlasm_register(register_line, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 47: JCL STEP Name Validator validation
    const char *step_line = "//MYSTEP1 EXEC PGM=IEFBR14\n";
    assert(tsfi_zorse_validate_jcl_stepname(step_line, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 48: COBOL PICTURE to VSAM Sizing Estimator validation
    int rec_len = 0;
    assert(tsfi_zorse_estimate_vsam_size("01 MY-REC PIC X(150).", &rec_len) == 0);
    assert(rec_len == 150);

    // Test Case 49: JCL COND Logic Evaluator validation
    int should_run = 0;
    assert(tsfi_zorse_evaluate_step_cond(8, "COND=(4,LT)", &should_run) == 0);
    // 8 < 4 is false -> should not bypass -> should_run = 1
    assert(should_run == 1);
    
    assert(tsfi_zorse_evaluate_step_cond(2, "COND=(4,LT)", &should_run) == 0);
    // 2 < 4 is true -> should bypass -> should_run = 0
    assert(should_run == 0);

    // Test Case 50: COBOL FILE STATUS Error Code Explainer validation
    assert(strcmp(tsfi_zorse_explain_file_status(35), "File not found") == 0);
    assert(strcmp(tsfi_zorse_explain_file_status(39), "Attribute conflict mismatch") == 0);

    // Test Case 51: JCL REGION Sizing Validator validation
    assert(tsfi_zorse_validate_jcl_region("//STEP1 EXEC PGM=IEFBR14,REGION=4M\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 52: COBOL OCCURS DEPENDING ON Clause Auditor validation
    const char *occurs_dep_inp = "05 MY-ARRAY PIC X OCCURS 1 TO 100 TIMES DEPENDING ON MY-COUNTER.";
    assert(tsfi_zorse_validate_cobol_occurs_depending(occurs_dep_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 53: JCL TIME Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_time("//STEP1 EXEC PGM=IEFBR14,TIME=1440\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 54: COBOL EVALUATE Statement Branch Auditor validation
    const char *eval_inp = "EVALUATE MY-VAL WHEN 1 PERFORM PROC-1 WHEN OTHER PERFORM PROC-OTHER END-EVALUATE.";
    assert(tsfi_zorse_validate_cobol_evaluate(eval_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 55: JCL MSGCLASS Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_msgclass("//MYJOB JOB MSGCLASS=A\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 56: COBOL PERFORM Statement Branch Auditor validation
    const char *perform_inp = "PERFORM VARYING I FROM 1 BY 1 UNTIL I > 10 DISPLAY I END-PERFORM.";
    assert(tsfi_zorse_validate_cobol_perform(perform_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 57: JCL TYPRUN Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_typrun("//MYJOB JOB TYPRUN=SCAN\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 58: COBOL SEARCH Table Branch Auditor validation
    const char *search_inp = "SEARCH MY-TABLE AT END DISPLAY 'NOT FOUND' WHEN MY-KEY = 10 DISPLAY 'FOUND' END-SEARCH.";
    assert(tsfi_zorse_validate_cobol_search(search_inp, &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 59: JCL ADDRSPC Storage Mode Auditor validation
    assert(tsfi_zorse_validate_jcl_addrspc("//STEP1 EXEC PGM=IEFBR14,ADDRSPC=VIRT\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 60: Moondream Ray-Tracer Render Artifact Auditor validation
    char artifact_buf[256];
    int art_res = tsfi_zorse_audit_render_artifacts("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", artifact_buf, sizeof(artifact_buf));
    assert(art_res == 0 || art_res == -2 || art_res == -1);

    // Test Case 61: Moondream Visual Camera Matrix Optimizer validation
    char matrix_buf[256];
    int matrix_res = tsfi_zorse_optimize_camera_matrix("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", matrix_buf, sizeof(matrix_buf));
    assert(matrix_res == 0 || matrix_res == -2 || matrix_res == -1);

    // Test Case 62: Moondream Material Properties Auditor validation
    char mat_buf[256];
    int mat_res = tsfi_zorse_audit_material_properties("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", mat_buf, sizeof(mat_buf));
    assert(mat_res == 0 || mat_res == -2 || mat_res == -1);

    // Test Case 63: Moondream Visual Bounding Box Collision Optimizer validation
    char col_buf[256];
    int col_res = tsfi_zorse_optimize_bounding_collision("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", col_buf, sizeof(col_buf));
    assert(col_res == 0 || col_res == -2 || col_res == -1);

    // Test Case 64: Moondream Visual Icon Transparency & Contrast Auditor validation
    char trans_buf[256];
    int trans_res = tsfi_zorse_audit_icon_transparency("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", trans_buf, sizeof(trans_buf));
    assert(trans_res == 0 || trans_res == -2 || trans_res == -1);

    // Test Case 65: Moondream Visual Icon Style Consistency Auditor validation
    char style_buf[256];
    int style_res = tsfi_zorse_audit_icon_style("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", style_buf, sizeof(style_buf));
    assert(style_res == 0 || style_res == -2 || style_res == -1);

    // Test Case 66: COBOL INITIALIZE Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_initialize("INITIALIZE MY-RECORD.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 67: JCL DSNTYPE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dsntype("//DD1 DD DSN=A.B.C,DSNTYPE=LIBRARY\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 68: COBOL GOTO Statement Path Auditor validation
    assert(tsfi_zorse_validate_cobol_goto("GO TO MY-PARA.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 69: JCL SMS Storage Class Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_sms("//DD2 DD DSN=A.B.C,DATACLAS=EXTENDED\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 70: COBOL STRING Statement Delimiter Auditor validation
    assert(tsfi_zorse_validate_cobol_string("STRING STR1 STR2 DELIMITED BY SIZE INTO STR3.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 71: JCL RECORG Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_recorg("//DD3 DD DSN=A.B.C,RECORG=KS\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 72: COBOL INSPECT TALLYING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_inspect("INSPECT MY-STR TALLYING MY-COUNT FOR ALL 'A'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 73: JCL AVGREC Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_avgrec("//DD4 DD DSN=A.B.C,AVGREC=U\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 74: COBOL UNSTRING Statement Pointer Auditor validation
    assert(tsfi_zorse_validate_cobol_unstring("UNSTRING MY-STR INTO DEST-STR1 DEST-STR2.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 75: JCL SECMODEL Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_secmodel("//DD5 DD DSN=A.B.C,SECMODEL=GDG\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 76: COBOL ADD SIZE ERROR Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_size_error("ADD A TO B ON SIZE ERROR DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 77: JCL DSORG Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dsorg("//DD6 DD DSN=A.B.C,DSORG=PS\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 78: Moondream Visual Icon Balance Auditor validation
    char bal_buf[256];
    int bal_res = tsfi_zorse_audit_icon_balance("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", bal_buf, sizeof(bal_buf));
    assert(bal_res == 0 || bal_res == -2 || bal_res == -1);

    // Test Case 79: Moondream Visual Icon Brand Palette Compliance Auditor validation
    char pal_buf[256];
    int pal_res = tsfi_zorse_audit_icon_palette("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", pal_buf, sizeof(pal_buf));
    assert(pal_res == 0 || pal_res == -2 || pal_res == -1);

    // Test Case 80: COBOL MULTIPLY SIZE ERROR Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_multiply_error("MULTIPLY A BY B ON SIZE ERROR DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 81: JCL LABEL Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_label("//DD7 DD DSN=A.B.C,LABEL=(,SL)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 82: Moondream Visual Light Bloom & Glare Auditor validation
    char bloom_buf[256];
    int bloom_res = tsfi_zorse_audit_light_bloom("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", bloom_buf, sizeof(bloom_buf));
    assert(bloom_res == 0 || bloom_res == -2 || bloom_res == -1);

    // Test Case 83: Moondream Visual Denoising Clarity Auditor validation
    char denoise_buf[256];
    int denoise_res = tsfi_zorse_audit_denoising_clarity("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", denoise_buf, sizeof(denoise_buf));
    assert(denoise_res == 0 || denoise_res == -2 || denoise_res == -1);

    // Test Case 84: COBOL DIVIDE SIZE ERROR Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_divide_error("DIVIDE A INTO B ON SIZE ERROR DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 85: JCL KEYLEN Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_keylen("//DD8 DD DSN=A.B.C,KEYLEN=8\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 86: COBOL SUBTRACT SIZE ERROR Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_subtract_error("SUBTRACT A FROM B ON SIZE ERROR DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 87: JCL EXPDT Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_expdt("//DD9 DD DSN=A.B.C,EXPDT=99365\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 88: COBOL COMPUTE SIZE ERROR Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_compute_error("COMPUTE A = B / C ON SIZE ERROR DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 89: JCL RETPD Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_retpd("//DD10 DD DSN=A.B.C,RETPD=30\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 90: COBOL WRITE INVALID KEY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_invalid_key("WRITE REC-OUT INVALID KEY DISPLAY 'ERROR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 91: JCL UCS Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_ucs("//PRT DD SYSOUT=A,UCS=AN\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 92: COBOL START INVALID KEY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_start_key("START MY-FILE KEY IS EQUAL TO MY-KEY INVALID KEY DISPLAY 'ERR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 93: JCL FCB Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_fcb("//PRT2 DD SYSOUT=A,FCB=STD1\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 94: COBOL READ INVALID KEY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_read_key("READ MY-FILE RECORD INTO MY-REC INVALID KEY DISPLAY 'ERR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 95: JCL PRTY Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_prty("//JOB1 JOB 'TEST',PRTY=8\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 96: COBOL DELETE INVALID KEY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_delete_key("DELETE MY-FILE RECORD INVALID KEY DISPLAY 'ERR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 97: JCL BURST Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_burst("//PRT3 DD SYSOUT=A,BURST=Y\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 98: COBOL REWRITE INVALID KEY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_rewrite_key("REWRITE REC-IN INVALID KEY DISPLAY 'ERR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 99: JCL FLASH Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_flash("//PRT4 DD SYSOUT=A,FLASH=STD1\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 100: COBOL CALL ON EXCEPTION Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_call_exception("CALL 'MYPROG' ON EXCEPTION DISPLAY 'ERR'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 101: JCL HOLD Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_hold("//DD11 DD DSN=A.B.C,HOLD=YES\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 102: COBOL ACCEPT Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_accept("ACCEPT CURRENT-DATE FROM DATE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 103: JCL FREE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_free("//DD12 DD DSN=A.B.C,FREE=CLOSE\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 104: COBOL DISPLAY Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_display("DISPLAY 'HELLO' UPON CONSOLE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 105: JCL OUTLIM Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_outlim("//DD13 DD SYSOUT=A,OUTLIM=5000\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 106: COBOL ENTRY Point Auditor validation
    assert(tsfi_zorse_validate_cobol_entry("ENTRY 'SUBENTRY' USING PARM-1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 107: JCL JOBLIB / STEPLIB Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_steplib("//STEPLIB DD DSN=MY.LOADLIB,DISP=SHR\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 108: COBOL GOBACK Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_goback("GOBACK.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 109: JCL DEST Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dest("//DD14 DD SYSOUT=A,DEST=RMT1\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 110: COBOL CANCEL Program Auditor validation
    assert(tsfi_zorse_validate_cobol_cancel("CANCEL 'SUBPROG'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 111: JCL EXPORT SYMLIST Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_export("//EXPORT EXPORT SYMLIST=(VAR1,VAR2)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 112: COBOL EXIT Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_exit("EXIT PROGRAM.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 113: JCL SET Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_set("// SET VAR1=VALUE1\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 114: COBOL CONTINUE Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_continue("CONTINUE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 115: JCL IMPORT Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_import("//IMPORT IMPORT SYMLIST=*\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 116: COBOL INITIALIZE REPLACING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_init_repl("INITIALIZE MY-RECORD REPLACING ALPHANUMERIC DATA BY SPACES.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 117: JCL JCLLIB Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_jcllib("//MYLIB JCLLIB ORDER=SYS1.PROCLIB\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 118: z/VSE CICS DFHRESP Response Auditor validation
    assert(tsfi_zorse_validate_vse_cics_dfhresp("IF EIBAID = DFHRESP(NORMAL) PERFORM GO-ON.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 119: z/VSE DL/I CBLTDLI Call Auditor validation
    assert(tsfi_zorse_validate_vse_dli_call("CALL 'CBLTDLI' USING GU-FUNC PCB-NAME.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 120: z/VSE POWER Spooling Parameter Auditor validation
    assert(tsfi_zorse_validate_vse_power_spool("* $$ LST CLASS=A,DISP=D\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 121: z/VSE VSAM Record-Level Sharing (RLS) Lock Auditor validation
    assert(tsfi_zorse_validate_vse_vsam_rls("//DD15 DD DSN=A.B.C,RLS=NRI\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 122: Zorse and z/VSE Combined DAT-based LLM Query validation
    char resp_buf[256];
    assert(tsfi_zorse_eval_dat_llm_query(NULL, "CBLTDLI", resp_buf, sizeof(resp_buf)) == 0);
    assert(strstr(resp_buf, "CBLTDLI") != NULL);

    // Test Case 123: z/VSE VTAM Multi-Path Channel (MPC) Auditor validation
    assert(tsfi_zorse_validate_vse_vtam_mpc("PATH01  LINE  UNIT=0E00,TYPE=MPC\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 124: z/VSE Virtual Disk (FBA) Parameter Auditor validation
    assert(tsfi_zorse_validate_vse_fba_vdisk("// VDISK UNIT=FBA,SIZE=20480\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 125: COBOL EVALUATE ALSO Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_evaluate_also("EVALUATE TRUE ALSO VAL-1", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 126: JCL VTAM PORT Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_vtam_port("PORT01   PORT  PORTNAME=GPPORT\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 127: VSEn Vaesen System Registry (Auncient Vaesen) validation
    remove("vaesen_registry.dat.bin");
    assert(tsfi_vsen_vaesen_register("Nacken", "Neck", 9, "Active") == 0);
    assert(tsfi_vsen_vaesen_register("Tomte", "Nisse", 3, "Friendly") == 0);
    
    char type_buf[64];
    char status_buf[32];
    int risk = 0;
    assert(tsfi_vsen_vaesen_lookup("Nacken", type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(type_buf, "Neck") == 0);
    assert(risk == 9);
    assert(strcmp(status_buf, "Active") == 0);
    
    int allowed = 0;
    assert(tsfi_vsen_vaesen_audit_transaction("TRX1", "Nacken", &allowed) == 0);
    assert(allowed == 0); // Risk level 9 > 8 should be flagged/disallowed
    
    assert(tsfi_vsen_vaesen_audit_transaction("TRX1", "Tomte", &allowed) == 0);
    assert(allowed == 1); // Risk level 3 <= 8 should be allowed
    
    remove("vaesen_registry.dat.bin");

    // Test Case 128: VSEn Vaesen Sight Telemetry Tracker validation
    remove("vaesen_sights.dat.bin");
    assert(tsfi_vsen_vaesen_record_sight("Nacken", "Upsala", 8) == 0);
    assert(tsfi_vsen_vaesen_record_sight("Tomte", "Upsala", 2) == 0);
    assert(tsfi_vsen_vaesen_record_sight("Vaettar", "Gothia", 4) == 0);
    
    int agg_fear = 0;
    assert(tsfi_vsen_vaesen_get_aggregate_fear("Upsala", &agg_fear) == 0);
    assert(agg_fear == 10); // 8 + 2
    
    assert(tsfi_vsen_vaesen_get_aggregate_fear("Gothia", &agg_fear) == 0);
    assert(agg_fear == 4);
    
    remove("vaesen_sights.dat.bin");

    // Test Case 129: COBOL CALL USING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_call_using("CALL 'SUBPROG' USING BY REFERENCE PARM-1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 130: JCL SYMLIST Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_symlist("//DD16 DD SYSOUT=*,SYMLIST=(VAR1)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 131: COBOL INSPECT REPLACING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_inspect_repl("INSPECT STR-IN REPLACING ALL 'A' BY 'B'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 132: JCL DLURVTAM Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dlurvtam("//DLUR01 DD DSN=SYS1.VTAMLST,DLURVTAM=YES\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 133: COBOL EXIT PROGRAM Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_exit_program("EXIT PROGRAM.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 134: JCL SYSOUT DEST Spool Router Auditor validation
    assert(tsfi_zorse_validate_jcl_dest_router("//DD17 DD SYSOUT=A,DEST=RMT12\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 135: COBOL STOP RUN Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_stop_run("STOP RUN.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 136: JCL DD DSID Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dsid("//DD18 DD DSID=SYSIN\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 137: COBOL GOTO DEPENDING ON Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_goto_depending("GO TO PROC-A PROC-B DEPENDING ON FLD-IDX.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 138: JCL DD DSNAME Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dsname("//DD19 DD DSNAME=MY.DATASET.VAL\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 139: COBOL SORT Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_sort("SORT WORK-FILE ON ASCENDING KEY WORK-KEY.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 140: JCL DD PATH Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_path("//DD20 DD PATH='/u/usr/data'\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 141: COBOL SEARCH ALL Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_search_all("SEARCH ALL TBL-ITEM WHEN ITEM-KEY(IDX) = VAL-IN", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 142: JCL DD GDG Generation Auditor validation
    assert(tsfi_zorse_validate_jcl_gdg("//DD21 DD DSN=APP.GDG.DATASET(+1),DISP=(NEW,CATLG)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 143: COBOL READ NEXT Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_read_next("READ FILE-A NEXT RECORD INTO REC-BUF.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 144: JCL DD PATHMODE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_pathmode("//DD22 DD PATH='/u/usr/data',PATHMODE=SIRWXU\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 145: COBOL WRITE FROM Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_write_from("WRITE REC-OUT FROM WORK-REC.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 146: JCL DD GDG Version Auditor validation
    assert(tsfi_zorse_validate_jcl_gdg_version("//DD23 DD DSN=APP.GDG.DATASET.G0001V00,DISP=SHR\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 147: Ray-Tracer Volumetric Scatter Auditor validation
    char vol_buf[1024];
    int vol_rc = tsfi_zorse_audit_volumetric_scatter("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", vol_buf, sizeof(vol_buf));
    assert(vol_rc == 0 || vol_rc == -1 || vol_rc == -2);

    // Test Case 148: Ray-Tracer Subsurface Translucency Auditor validation
    char sub_buf[1024];
    int sub_rc = tsfi_zorse_audit_subsurface_translucency("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", "moondream", sub_buf, sizeof(sub_buf));
    assert(sub_rc == 0 || sub_rc == -1 || sub_rc == -2);

    // Test Case 149: z/VSE Subsystem Support Services (Triple S Class) Auditor validation
    assert(tsfi_zorse_validate_vse_triple_s("// ASSIGN SYS005,READER,SSS=YES\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 150: COBOL STRING WITH POINTER Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_string_pointer("STRING STR-A STR-B DELIMITED BY SIZE INTO STR-OUT WITH POINTER PTR-VAR.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 151: JCL DD PATHDISP Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_pathdisp("//DD24 DD PATH='/tmp/out',PATHDISP=(KEEP,DELETE)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 152: COBOL INITIALIZE WITH FILLER Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_init_filler("INITIALIZE GRP-REC WITH FILLER.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 153: JCL DD FILEDATA Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_filedata("//DD25 DD PATH='/tmp/data.txt',FILEDATA=TEXT\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 154: COBOL ENTRY USING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_entry_using("ENTRY 'SUBPROG' USING PARM-1 PARM-2.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 155: JCL DD PROTECT Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_protect("//DD26 DD DSN=SECRET.DATA,PROTECT=YES\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 156: COBOL INSPECT TALLYING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_inspect_tallying("INSPECT STR-A TALLYING CHAR-CNT FOR ALL CHARACTERS.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 157: JCL DD SUBSYS Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_subsys("//DD27 DD SUBSYS=(LAM,PARM1)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 158: COBOL INSPECT TALLYING REPLACING Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_inspect_tally_repl("INSPECT STR-A TALLYING CHAR-CNT REPLACING ALL 'A' BY 'B'.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 159: JCL DD FREE=CLOSE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_free_close("//DD28 DD SYSOUT=A,FREE=CLOSE\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 160: COBOL COPY OF Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_copy_of("COPY MEMBER-A OF COPY-LIB.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 161: JCL DD ACCODE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_accode("//DD29 DD DSN=TAPE.DATA,ACCODE=SECURE1\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 162: SVDAG Vaesen parameters resolver validation
    float melanin = 0.1f;
    float roughness = 0.2f;
    float iridescence = 0.3f;
    extern int tsfi_vsen_vaesen_register(const char *name, const char *type, int risk_level, const char *status);
    extern int tsfi_vsen_vaesen_record_sight(const char *name, const char *location, int fear_level);
    assert(tsfi_vsen_vaesen_register("Mara", "Nightmare", 2, "Active") == 0);
    assert(tsfi_vsen_vaesen_record_sight("Mara", "Uppsala", 5) == 0);

    extern int tsfi_svdag_apply_vaesen_parameters(const char *vaesen_name, const char *region_name, float *melanin, float *roughness, float *iridescence);
    assert(tsfi_svdag_apply_vaesen_parameters("Mara", "Uppsala", &melanin, &roughness, &iridescence) == 0);
    // Mara (risk_level=2) -> melanin=0.1+0.2=0.3, roughness=0.2+0.1=0.3
    // Uppsala (fear_level=5) -> iridescence=0.3+0.4=0.7
    assert(melanin > 0.29f && melanin < 0.31f);
    assert(roughness > 0.29f && roughness < 0.31f);
    assert(iridescence > 0.69f && iridescence < 0.71f);

    remove("vaesen_registry.dat.bin");
    remove("vaesen_sights.dat.bin");

    // Test Case 163: COBOL XML GENERATE Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_xml_gen("XML GENERATE XML-DOC FROM GRP-REC.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 164: JCL DD DSNTYPE=LARGE Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_dsntype_large("//DD30 DD DSN=LARGE.DATA,DSNTYPE=LARGE\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 165: Zorse SVDAG compiler validation
    TSFiTasteTree *taste_tree = tsfi_svdag_create(10);
    assert(taste_tree != NULL);
    assert(tsfi_svdag_compile_zorse(taste_tree, "XML GENERATE DOC FROM REC.\nINSPECT STR.\n") == 0);
    assert(taste_tree->stream_size == 2);
    assert(taste_tree->intensity_stream[0] > 0.89f && taste_tree->intensity_stream[0] < 0.91f);
    tsfi_svdag_destroy(taste_tree);

    // Test Case 166: Zorse Stable Diffusion batch pipeline compliance auditor validation
    assert(tsfi_zorse_validate_stable_diffusion("//STEP EXEC PGM=SD_DIFFUSE\n", "LINKAGE SECTION. CALL USING PARM-1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 167: Zorse Stable Diffusion Algol phase variance simulation validation
    double noise_out = 0.0;
    assert(tsfi_zorse_execute_stable_diffusion_algol(2.0, 10.0, 1000.0, &noise_out) == 0);
    // 2^10 = 1024. 1024 mod 1000 = 24.
    assert(noise_out > 23.9f && noise_out < 24.1f);

    // Test Case 168: COBOL XML PARSE Statement Auditor validation
    assert(tsfi_zorse_validate_cobol_xml_parse("XML PARSE XML-TEXT PROCESSING PROCEDURE PARSE-HANDLER.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 169: JCL DD EROPT Parameter Auditor validation
    assert(tsfi_zorse_validate_jcl_eropt("//DD31 DD DSN=SYS.DATA,EROPT=ACC\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 170: SVDAG image voxelizer validation
    uint32_t mock_pixels[4] = {
        0xFFFFFFFF, 0x00000000,
        0x00000000, 0xFFFFFFFF
    };
    TSFiTasteTree *vox_tree = tsfi_svdag_create(10);
    assert(vox_tree != NULL);
    assert(tsfi_svdag_voxelize_image(vox_tree, mock_pixels, 2, 2, 0.5f) == 0);
    // Two bright pixels should pass the 0.5 threshold
    assert(vox_tree->stream_size == 2);
    assert(vox_tree->intensity_stream[0] > 0.99f);
    tsfi_svdag_destroy(vox_tree);

    // Test Case 171: z/VSE POWER submit compliance auditor validation
    assert(tsfi_zorse_validate_vse_power_submit("* $$ POWER SUBMIT\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 172: JCL DD DSNAME versioning compliance auditor validation
    assert(tsfi_zorse_validate_jcl_dsname_ver("//DD32 DD DSNAME=GDG.NAME.G0001V00,DISP=SHR\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 173: GDG name resolver validation
    char resolved_gdg[128];
    assert(tsfi_zorse_resolve_gdg_relative_to_absolute("ZORSE.GDG", 1, 4, resolved_gdg, sizeof(resolved_gdg)) == 0);
    assert(strcmp(resolved_gdg, "ZORSE.GDG.G0005V00") == 0);

    // Test Case 174: ZMM VM GDG Operating Context commit and rollback validation
    TsfiZmmVmState vm_state;
    tsfi_zmm_vm_init(&vm_state);
    vm_state.program_counter = 42;
    vm_state.llm_tx_counter = 1001;
    assert(tsfi_zmm_vm_commit_gdg_generation(&vm_state, "ZMM.STATE", 5) == 0);

    // Modify state
    vm_state.program_counter = 99;
    vm_state.llm_tx_counter = 8888;

    // Rollback
    assert(tsfi_zmm_vm_rollback_gdg_generation(&vm_state, "ZMM.STATE", 5) == 0);
    assert(vm_state.program_counter == 42);
    assert(vm_state.llm_tx_counter == 1001);
    tsfi_zmm_vm_destroy(&vm_state);

    // Cleanup the generated catalog file
    remove("ZMM.STATE.G0005V00.dat.bin");

    // Test Case 175: z/VSE CICS RDO Compliance Auditor validation
    assert(tsfi_zorse_validate_vse_cics_rdo("DEFINE CONNECTION(C01) NETNAME(N01) ACCESSMETHOD(VTAM)", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 176: COBOL Dynamic Call Auditor validation
    assert(tsfi_zorse_validate_cobol_dyn_call("CALL PROGRAM-ID-VAR USING PARM-1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 177: Red DDL Strategy Language Auditor validation
    assert(tsfi_zorse_validate_red_ddl("RED DDL: DEFINE STRATEGY FOR VAESEN", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 178: Black DML Data Model Auditor validation
    assert(tsfi_zorse_validate_black_dml("BLACK DML: STREAM IMAGE TO VRAM", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 179: JCL EXEC PGM Parameter Override Auditor validation
    assert(tsfi_zorse_validate_jcl_parm_override("//STEP1 EXEC PGM=IEFBR14,PARM='DEBUG=YES'\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 180: JCL SYSOUT Routing Auditor validation
    assert(tsfi_zorse_validate_jcl_sysout_route("//OUT1 DD SYSOUT=(A,INTRDR)\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 181: z/VSE POWER Spool Queue Class Validator validation
    assert(tsfi_zorse_validate_vse_power_class("* $$ LST CLASS=A,DISP=D\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 182: COBOL Internal Sorting Auditor validation
    assert(tsfi_zorse_validate_cobol_sort_key("SORT WORK-FILE ON ASCENDING KEY WORK-KEY.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 183: z/VSE POWER Spool Print Class Validator validation
    assert(tsfi_zorse_validate_vse_power_prtclass("* $$ LST PRTCLASS=A,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 184: COBOL File Merge Auditor validation
    assert(tsfi_zorse_validate_cobol_merge_key("MERGE OUT-FILE ON ASCENDING KEY OUT-KEY.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 185: z/VSE POWER Spool Punch Class Validator validation
    assert(tsfi_zorse_validate_vse_power_punclass("* $$ PUN PUNCLASS=B,DISP=K\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 186: COBOL Report Writer Auditor validation
    assert(tsfi_zorse_validate_cobol_generate("GENERATE SALES-REPORT.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 187: z/VSE POWER Spool Queue Hold Validator validation
    assert(tsfi_zorse_validate_vse_power_hold("* $$ LST HOLD=YES,DISP=D\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 188: COBOL Report Writer Initiation Auditor validation
    assert(tsfi_zorse_validate_cobol_initiate("INITIATE SALES-REPORT.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 189: z/VSE POWER Spool Queue Disposition Validator validation
    assert(tsfi_zorse_validate_vse_power_disp("* $$ LST DISP=D,CLASS=A\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 190: COBOL Report Writer Termination Auditor validation
    assert(tsfi_zorse_validate_cobol_terminate("TERMINATE SALES-REPORT.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 191: z/VSE POWER Spool Print Class Override Validator validation
    assert(tsfi_zorse_validate_vse_power_prtclass_override("* $$ LST PRTCLASS=(A,B),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 192: COBOL Report Writer Page Heading Auditor validation
    assert(tsfi_zorse_validate_cobol_page_heading("PAGE HEADING. 01 TITLE-LINE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 193: z/VSE POWER Spool Print Copies Validator validation
    assert(tsfi_zorse_validate_vse_power_copies("* $$ LST COPIES=3,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 194: COBOL Report Writer Page Limit Auditor validation
    assert(tsfi_zorse_validate_cobol_page_limit("PAGE LIMIT 66 LINES.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 195: z/VSE POWER Spool Page Range Limit Validator validation
    assert(tsfi_zorse_validate_vse_power_pages("* $$ LST PAGES=(1,10),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 196: COBOL Report Writer Heading Group Auditor validation
    assert(tsfi_zorse_validate_cobol_heading_group("01 TYPE IS PAGE HEADING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 197: z/VSE POWER Spool Page Override Validator validation
    assert(tsfi_zorse_validate_vse_power_pages_override("* $$ LST PAGES=(1,5,10),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 198: COBOL Report Writer Detail Group Auditor validation
    assert(tsfi_zorse_validate_cobol_detail_group("01 TYPE IS DETAIL.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 199: z/VSE POWER Spool Printer Forms Validator validation
    assert(tsfi_zorse_validate_vse_power_form("* $$ LST FORM=STD,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 200: COBOL Report Writer Control Heading Auditor validation
    assert(tsfi_zorse_validate_cobol_control_heading("01 TYPE IS CONTROL HEADING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 201: z/VSE POWER Spool Print Line Limit Validator validation
    assert(tsfi_zorse_validate_vse_power_lines("* $$ LST LINES=1000,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 202: COBOL Report Writer Control Footing Auditor validation
    assert(tsfi_zorse_validate_cobol_control_footing("01 TYPE IS CONTROL FOOTING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 203: z/VSE POWER Spool Page Range List Validator validation
    assert(tsfi_zorse_validate_vse_power_pages_list("* $$ LST PAGES=(1,2,5),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 204: COBOL Report Writer Page Footing Auditor validation
    assert(tsfi_zorse_validate_cobol_page_footing("01 TYPE IS PAGE FOOTING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 205: z/VSE POWER Spool Page Count Limit Validator validation
    assert(tsfi_zorse_validate_vse_power_pages_limit("* $$ LST PAGES=10,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 206: COBOL Report Writer Report Heading Auditor validation
    assert(tsfi_zorse_validate_cobol_report_heading("01 TYPE IS REPORT HEADING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 207: z/VSE POWER Spool Print Node Destination Validator validation
    assert(tsfi_zorse_validate_vse_power_node("* $$ LST NODE=NODE1,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 208: COBOL Report Writer Report Footing Auditor validation
    assert(tsfi_zorse_validate_cobol_report_footing("01 TYPE IS REPORT FOOTING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 209: z/VSE POWER Spool Print Forms Overlay Validator validation
    assert(tsfi_zorse_validate_vse_power_flash("* $$ LST FLASH=OVL1,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 210: COBOL Report Writer Control Footing Group Auditor validation
    assert(tsfi_zorse_validate_cobol_cf_group("01 TYPE IS CONTROL FOOTING.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 211: z/VSE POWER Spool Forms Flash Override Validator validation
    assert(tsfi_zorse_validate_vse_power_flash_override("* $$ LST FLASH=(OVL1,OVL2),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 212: COBOL Report Writer Detail Group Line Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_line_spacing("01 DETAIL-LINE LINE PLUS 1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 213: z/VSE POWER Spool forms flash overlay Status Validator validation
    assert(tsfi_zorse_validate_vse_power_flash_status("* $$ LST FLASH=(OVL1,STATUS),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 214: COBOL Report Writer Next Page Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_next_page("01 DETAIL-LINE LINE NEXT PAGE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 215: z/VSE POWER Spool Destination Node Status Validator validation
    assert(tsfi_zorse_validate_vse_power_node_status("* $$ LST NODE=(NODE1,STATUS),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 216: COBOL Report Writer Next Page Limit Auditor validation
    assert(tsfi_zorse_validate_cobol_next_page_limit("01 DETAIL-LINE LINE NEXT PAGE LIMIT 50.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 217: z/VSE POWER Spool Destination Node Override Validator validation
    assert(tsfi_zorse_validate_vse_power_node_override("* $$ LST NODE=(NODE1,NODE2),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 218: COBOL Report Writer Control Footing Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_cf_spacing("01 CF-LINE CONTROL FOOTING LINE PLUS 2.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 219: z/VSE POWER Spool Print Priority Validator validation
    assert(tsfi_zorse_validate_vse_power_pri("* $$ LST PRI=3,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 220: COBOL Report Writer Page Footing Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_pf_spacing("01 PF-LINE PAGE FOOTING LINE PLUS 1.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 221: z/VSE POWER Spool Print Priority Override Validator validation
    assert(tsfi_zorse_validate_vse_power_pri_override("* $$ LST PRI=(3,5),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 222: COBOL Report Writer Report Heading Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_rh_spacing("01 RH-LINE REPORT HEADING LINE PLUS 3.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 223: z/VSE POWER Spool Print Retention Validator validation
    assert(tsfi_zorse_validate_vse_power_keep("* $$ LST KEEP=YES,DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 224: COBOL Report Writer Final Summary Group Auditor validation
    assert(tsfi_zorse_validate_cobol_final_summary("01 SUMMARY-LINE CONTROL FOOTING FINAL.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 225: z/VSE POWER Spool Print Retention Status Validator validation
    assert(tsfi_zorse_validate_vse_power_keep_status("* $$ LST KEEP=(YES,STATUS),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 226: COBOL Report Writer Final Summary Spacing Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing("01 SUMMARY-LINE FINAL LINE PLUS 2.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 227: z/VSE POWER Spool Print Class Override Validator validation
    assert(tsfi_zorse_validate_vse_power_class_override("* $$ LST CLASS=(A,B),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 228: COBOL Report Writer Page Heading Spacing Limit Auditor validation
    assert(tsfi_zorse_validate_cobol_ph_spacing_limit("01 PH-LINE PAGE HEADING LIMIT 60.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 229: z/VSE POWER Spool Print Class Status Validator validation
    assert(tsfi_zorse_validate_vse_power_class_status("* $$ LST CLASS=(A,STATUS),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 230: COBOL Report Writer Page Footing Spacing Limit Auditor validation
    assert(tsfi_zorse_validate_cobol_pf_spacing_limit("01 PF-LINE PAGE FOOTING LIMIT 55.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 231: z/VSE POWER Spool Print Retention Override Validator validation
    assert(tsfi_zorse_validate_vse_power_keep_override("* $$ LST KEEP=(YES,NO),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 232: COBOL Report Writer Final Summary Spacing Limit Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing_limit("01 SUMMARY-LINE FINAL LIMIT 65.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 233: z/VSE POWER Spool Print Retention Status Override Validator validation
    assert(tsfi_zorse_validate_vse_power_keep_status_override("* $$ LST KEEP=(YES,STATUS,OVERRIDE),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 234: COBOL Report Writer Final Summary Spacing Limit Override Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing_limit_override("01 SUMMARY-LINE FINAL LIMIT 70 OVERRIDE.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 235: z/VSE POWER Spool Print Priority Status Override Validator validation
    assert(tsfi_zorse_validate_vse_power_pri_status_override("* $$ LST PRI=(3,STATUS,OVERRIDE),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 236: COBOL Report Writer Final Summary Spacing Limit Override Checks Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing_checks("01 SUMMARY-LINE FINAL LIMIT 75 OVERRIDE CHECKS.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 237: z/VSE POWER Spool Print Priority Override List Validator validation
    assert(tsfi_zorse_validate_vse_power_pri_override_list("* $$ LST PRI=(3,5),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 238: COBOL Report Writer Final Summary Spacing Limit Override Checks List Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing_list_checks("01 SUMMARY-LINE FINAL LIMIT 80 OVERRIDE CHECKS LIST.", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 239: z/VSE POWER Spool Print Priority Status Override List Checks Validator validation
    assert(tsfi_zorse_validate_vse_power_pri_status_checks("* $$ LST PRI=(3,STATUS,OVERRIDE,CHECKS),DISP=H\n", &is_valid) == 0);
    assert(is_valid == 1);

    // Test Case 240: COBOL Report Writer Final Summary Spacing Limit Override Checks List Status Auditor validation
    assert(tsfi_zorse_validate_cobol_final_spacing_status("01 SUMMARY-LINE FINAL LIMIT 85 OVERRIDE CHECKS LIST STATUS.", &is_valid) == 0);
    assert(is_valid == 1);

    printf("[PASS] Zorse compliance evaluation tests verified successfully!\n");
    return 0;
}
