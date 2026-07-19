#ifndef TSFI_ZORSE_EVAL_H
#define TSFI_ZORSE_EVAL_H

#include <stddef.h>

// Zorse JCL Syntax Compliance Validator
int tsfi_zorse_validate_jcl(const char *jcl_string, int *is_valid_out, char *error_msg_out, int max_err_len);

// Zorse COBOL Division Structure Compliance Validator
int tsfi_zorse_validate_cobol(const char *cobol_string, int *is_valid_out);

// Zorse JCL DD Parameter Auditor
int tsfi_zorse_validate_jcl_dd(const char *dd_statement, int *is_valid_out);

// Zorse HLASM Instruction Compliance Auditor
int tsfi_zorse_validate_hlasm(const char *hlasm_instruction, int *is_valid_out);

// Zorse LLM Integration Query Interface
int tsfi_zorse_query_llm(const char *prompt, const char *model_name, char *response_out, size_t max_resp_len);

// Zorse COBOL PICTURE Clause Auditor
int tsfi_zorse_validate_cobol_pic(const char *pic_clause, int *is_valid_out);

// Zorse JCL EXEC Program Validator
int tsfi_zorse_validate_jcl_pgm(const char *exec_statement, int *is_valid_out);

// Zorse COBOL REDEFINES Clause Auditor
int tsfi_zorse_validate_cobol_redefines(const char *redefines_clause, int *is_valid_out);

// Zorse JCL Symbolic Parameter Validator
int tsfi_zorse_validate_jcl_symbolic(const char *jcl_line, int *is_valid_out);

// Zorse COBOL OCCURS Clause Auditor
int tsfi_zorse_validate_cobol_occurs(const char *occurs_clause, int *is_valid_out);

// Zorse JCL COND Parameter Validator
int tsfi_zorse_validate_jcl_cond(const char *cond_parameter, int *is_valid_out);

// Zorse Self-Correcting Syntax Pipeline
int tsfi_zorse_autocorrect_source(const char *failed_source, const char *lang, const char *model_name, char *corrected_source_out, size_t max_len);

// Zorse LLM COBOL-to-JCL Dependency Resolver
int tsfi_zorse_resolve_dependencies(const char *cobol_src, const char *jcl_src, const char *model_name, char *mapping_out, size_t max_len);

// Zorse Moondream Visual Console Auditor
int tsfi_zorse_audit_screen_visual(const char *b64_screen_img, const char *model_name, char *alert_level_out, size_t max_len);

// Zorse LLM COBOL-to-HLASM Transpiler
int tsfi_zorse_transpile_cobol_to_hlasm(const char *cobol_src, const char *model_name, char *hlasm_out, size_t max_len);

// Zorse Moondream Flowchart-to-JCL Generator
int tsfi_zorse_generate_jcl_from_flowchart(const char *b64_flowchart_img, const char *model_name, char *jcl_out, size_t max_len);

// Zorse LLM HLASM-to-COBOL Reverse Transpiler
int tsfi_zorse_transpile_hlasm_to_cobol(const char *hlasm_src, const char *model_name, char *cobol_out, size_t max_len);

// Zorse Moondream DASD Layout-to-JCL Space Mapper
int tsfi_zorse_map_dasd_space(const char *b64_layout_img, const char *model_name, char *space_out, size_t max_len);

// Zorse LLM JCL/COBOL Code Explain-Pipeline
int tsfi_zorse_explain_source(const char *source, const char *lang, const char *model_name, char *explanation_out, size_t max_len);

// Zorse Moondream Visual Punch Card Reader
int tsfi_zorse_read_punch_card(const char *b64_card_img, const char *model_name, char *text_out, size_t max_len);

// Zorse Moondream Tape Drive Mount Auditor
int tsfi_zorse_audit_tape_mount(const char *b64_tape_img, const char *expected_tape_id, const char *model_name, int *is_mounted_out);

// Zorse Moondream Cabling Topology Parser
int tsfi_zorse_parse_cabling_topology(const char *b64_topology_img, const char *model_name, char *sna_config_out, size_t max_len);

// Zorse Moondream Cabinet Thermal Graph Auditor
int tsfi_zorse_audit_thermal_graph(const char *b64_thermal_img, const char *model_name, char *hotspot_info_out, size_t max_len);

// Zorse Mainframe Job Stream Orchestrator
int tsfi_zorse_audit_job_stream(const char *jcl_source, const char *cobol_source, const char *model_name, int *is_valid_out, char *report_out, size_t max_report_len);

// Zorse LLM COBOL-to-C Struct Transpiler
int tsfi_zorse_transpile_cobol_to_c(const char *cobol_data_div, const char *model_name, char *c_struct_out, size_t max_len);

// Zorse SNA Session Protocol Auditor
int tsfi_zorse_audit_sna_session(const char *sna_bind_hex, int *is_valid_out);

// Zorse Moondream Cabinet Fan Acoustic Spectrogram Auditor
int tsfi_zorse_audit_fan_spectrogram(const char *b64_spectrogram_img, const char *model_name, char *audit_result_out, size_t max_len);

// Zorse COBOL-to-JCL SPACE Allocation Optimizer
int tsfi_zorse_optimize_jcl_space(const char *cobol_src, const char *model_name, char *space_opt_out, size_t max_len);

// Zorse COBOL Copybook Dependency Resolver
int tsfi_zorse_resolve_copybooks(const char *cobol_src, char *dependencies_out, size_t max_len);

// Zorse JCL DD DISP Parameter Auditor
int tsfi_zorse_validate_jcl_disp(const char *dd_statement, int *is_valid_out);

// Zorse HLASM Macro Definition Parser
int tsfi_zorse_validate_hlasm_macro(const char *macro_src, int *is_valid_out);

// Zorse JCL Step Flow Visualizer Prompt Generator
int tsfi_zorse_generate_flow_prompt(const char *jcl_src, const char *model_name, char *prompt_out, size_t max_len);

// Zorse COBOL FILE STATUS Logic Auditor
int tsfi_zorse_validate_cobol_file_status(const char *cobol_src, int *is_valid_out);

// Zorse JCL SYSOUT Class Auditor
int tsfi_zorse_validate_jcl_sysout(const char *jcl_line, int *is_valid_out);

// Zorse HLASM Register Usage Tracker
int tsfi_zorse_validate_hlasm_register(const char *hlasm_line, int *is_valid_out);

// Zorse JCL STEP Name Validator
int tsfi_zorse_validate_jcl_stepname(const char *jcl_line, int *is_valid_out);

// Zorse COBOL PICTURE to VSAM Sizing Estimator
int tsfi_zorse_estimate_vsam_size(const char *cobol_pic_clause, int *record_len_out);

// Zorse JCL COND Logic Evaluator
int tsfi_zorse_evaluate_step_cond(int step_rc, const char *cond_expr, int *should_run_out);

#endif // TSFI_ZORSE_EVAL_H
