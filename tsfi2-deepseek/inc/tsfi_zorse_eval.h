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

// Zorse COBOL FILE STATUS Error Code Explainer
const char* tsfi_zorse_explain_file_status(int status_code);

// Zorse JCL REGION Sizing Validator
int tsfi_zorse_validate_jcl_region(const char *jcl_line, int *is_valid_out);

// Zorse COBOL OCCURS DEPENDING ON Clause Auditor
int tsfi_zorse_validate_cobol_occurs_depending(const char *occurs_clause, int *is_valid_out);

// Zorse JCL TIME Parameter Auditor
int tsfi_zorse_validate_jcl_time(const char *jcl_line, int *is_valid_out);

// Zorse COBOL EVALUATE Statement Branch Auditor
int tsfi_zorse_validate_cobol_evaluate(const char *cobol_src, int *is_valid_out);

// Zorse JCL MSGCLASS Parameter Auditor
int tsfi_zorse_validate_jcl_msgclass(const char *jcl_line, int *is_valid_out);

// Zorse COBOL PERFORM Statement Branch Auditor
int tsfi_zorse_validate_cobol_perform(const char *cobol_src, int *is_valid_out);

// Zorse JCL TYPRUN Parameter Auditor
int tsfi_zorse_validate_jcl_typrun(const char *jcl_line, int *is_valid_out);

// Zorse COBOL SEARCH Statement Branch Auditor
int tsfi_zorse_validate_cobol_search(const char *cobol_src, int *is_valid_out);

// Zorse JCL ADDRSPC Storage Mode Auditor
int tsfi_zorse_validate_jcl_addrspc(const char *jcl_line, int *is_valid_out);

// Zorse Moondream Ray-Tracer Render Artifact Auditor
int tsfi_zorse_audit_render_artifacts(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Camera Matrix Optimizer
int tsfi_zorse_optimize_camera_matrix(const char *b64_render_img, const char *model_name, char *matrix_out, size_t max_len);

// Zorse Moondream Ray-Tracer Material Properties Auditor
int tsfi_zorse_audit_material_properties(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Bounding Box Collision Optimizer
int tsfi_zorse_optimize_bounding_collision(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Icon Transparency & Contrast Auditor
int tsfi_zorse_audit_icon_transparency(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Icon Style Consistency Auditor
int tsfi_zorse_audit_icon_style(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse COBOL INITIALIZE Statement Auditor
int tsfi_zorse_validate_cobol_initialize(const char *cobol_src, int *is_valid_out);

// Zorse JCL DSNTYPE Parameter Auditor
int tsfi_zorse_validate_jcl_dsntype(const char *jcl_line, int *is_valid_out);

// Zorse COBOL GOTO Statement Path Auditor
int tsfi_zorse_validate_cobol_goto(const char *cobol_src, int *is_valid_out);

// Zorse JCL SMS Storage Class Parameter Auditor
int tsfi_zorse_validate_jcl_sms(const char *jcl_line, int *is_valid_out);

// Zorse COBOL STRING Statement Delimiter Auditor
int tsfi_zorse_validate_cobol_string(const char *cobol_src, int *is_valid_out);

// Zorse JCL RECORG Parameter Auditor
int tsfi_zorse_validate_jcl_recorg(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INSPECT TALLYING Statement Auditor
int tsfi_zorse_validate_cobol_inspect(const char *cobol_src, int *is_valid_out);

// Zorse JCL AVGREC Parameter Auditor
int tsfi_zorse_validate_jcl_avgrec(const char *jcl_line, int *is_valid_out);

// Zorse COBOL UNSTRING Statement Pointer Auditor
int tsfi_zorse_validate_cobol_unstring(const char *cobol_src, int *is_valid_out);

// Zorse JCL SECMODEL Parameter Auditor
int tsfi_zorse_validate_jcl_secmodel(const char *jcl_line, int *is_valid_out);

// Zorse COBOL ADD SIZE ERROR Statement Auditor
int tsfi_zorse_validate_cobol_size_error(const char *cobol_src, int *is_valid_out);

// Zorse JCL DSORG Parameter Auditor
int tsfi_zorse_validate_jcl_dsorg(const char *jcl_line, int *is_valid_out);

// Zorse Moondream Visual Icon Balance Auditor
int tsfi_zorse_audit_icon_balance(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Icon Brand Palette Compliance Auditor
int tsfi_zorse_audit_icon_palette(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse COBOL MULTIPLY SIZE ERROR Statement Auditor
int tsfi_zorse_validate_cobol_multiply_error(const char *cobol_src, int *is_valid_out);

// Zorse JCL LABEL Parameter Auditor
int tsfi_zorse_validate_jcl_label(const char *jcl_line, int *is_valid_out);

// Zorse Moondream Visual Light Bloom & Glare Auditor
int tsfi_zorse_audit_light_bloom(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse Moondream Visual Denoising Clarity Auditor
int tsfi_zorse_audit_denoising_clarity(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len);

// Zorse COBOL DIVIDE SIZE ERROR Statement Auditor
int tsfi_zorse_validate_cobol_divide_error(const char *cobol_src, int *is_valid_out);

// Zorse JCL KEYLEN Parameter Auditor
int tsfi_zorse_validate_jcl_keylen(const char *jcl_line, int *is_valid_out);

// Zorse COBOL SUBTRACT SIZE ERROR Statement Auditor
int tsfi_zorse_validate_cobol_subtract_error(const char *cobol_src, int *is_valid_out);

// Zorse JCL EXPDT Parameter Auditor
int tsfi_zorse_validate_jcl_expdt(const char *jcl_line, int *is_valid_out);

// Zorse COBOL COMPUTE SIZE ERROR Statement Auditor
int tsfi_zorse_validate_cobol_compute_error(const char *cobol_src, int *is_valid_out);

// Zorse JCL RETPD Parameter Auditor
int tsfi_zorse_validate_jcl_retpd(const char *jcl_line, int *is_valid_out);

// Zorse COBOL WRITE INVALID KEY Statement Auditor
int tsfi_zorse_validate_cobol_invalid_key(const char *cobol_src, int *is_valid_out);

// Zorse JCL UCS Parameter Auditor
int tsfi_zorse_validate_jcl_ucs(const char *jcl_line, int *is_valid_out);

// Zorse COBOL START INVALID KEY Statement Auditor
int tsfi_zorse_validate_cobol_start_key(const char *cobol_src, int *is_valid_out);

// Zorse JCL FCB Parameter Auditor
int tsfi_zorse_validate_jcl_fcb(const char *jcl_line, int *is_valid_out);

// Zorse COBOL READ INVALID KEY Statement Auditor
int tsfi_zorse_validate_cobol_read_key(const char *cobol_src, int *is_valid_out);

// Zorse JCL PRTY Parameter Auditor
int tsfi_zorse_validate_jcl_prty(const char *jcl_line, int *is_valid_out);

// Zorse COBOL DELETE INVALID KEY Statement Auditor
int tsfi_zorse_validate_cobol_delete_key(const char *cobol_src, int *is_valid_out);

// Zorse JCL BURST Parameter Auditor
int tsfi_zorse_validate_jcl_burst(const char *jcl_line, int *is_valid_out);

// Zorse COBOL REWRITE INVALID KEY Statement Auditor
int tsfi_zorse_validate_cobol_rewrite_key(const char *cobol_src, int *is_valid_out);

// Zorse JCL FLASH Parameter Auditor
int tsfi_zorse_validate_jcl_flash(const char *jcl_line, int *is_valid_out);

// Zorse COBOL CALL ON EXCEPTION Statement Auditor
int tsfi_zorse_validate_cobol_call_exception(const char *cobol_src, int *is_valid_out);

// Zorse JCL HOLD Parameter Auditor
int tsfi_zorse_validate_jcl_hold(const char *jcl_line, int *is_valid_out);

// Zorse COBOL ACCEPT Statement Auditor
int tsfi_zorse_validate_cobol_accept(const char *cobol_src, int *is_valid_out);

// Zorse JCL FREE Parameter Auditor
int tsfi_zorse_validate_jcl_free(const char *jcl_line, int *is_valid_out);

// Zorse COBOL DISPLAY Statement Auditor
int tsfi_zorse_validate_cobol_display(const char *cobol_src, int *is_valid_out);

// Zorse JCL OUTLIM Parameter Auditor
int tsfi_zorse_validate_jcl_outlim(const char *jcl_line, int *is_valid_out);

// Zorse COBOL ENTRY Point Auditor
int tsfi_zorse_validate_cobol_entry(const char *cobol_src, int *is_valid_out);

// Zorse JCL JOBLIB / STEPLIB Parameter Auditor
int tsfi_zorse_validate_jcl_steplib(const char *jcl_line, int *is_valid_out);

// Zorse COBOL GOBACK Statement Auditor
int tsfi_zorse_validate_cobol_goback(const char *cobol_src, int *is_valid_out);

// Zorse JCL DEST Parameter Auditor
int tsfi_zorse_validate_jcl_dest(const char *jcl_line, int *is_valid_out);

// Zorse COBOL CANCEL Program Auditor
int tsfi_zorse_validate_cobol_cancel(const char *cobol_src, int *is_valid_out);

// Zorse JCL EXPORT SYMLIST Parameter Auditor
int tsfi_zorse_validate_jcl_export(const char *jcl_line, int *is_valid_out);

// Zorse COBOL EXIT Statement Auditor
int tsfi_zorse_validate_cobol_exit(const char *cobol_src, int *is_valid_out);

// Zorse JCL SET Parameter Auditor
int tsfi_zorse_validate_jcl_set(const char *jcl_line, int *is_valid_out);

// Zorse COBOL CONTINUE Statement Auditor
int tsfi_zorse_validate_cobol_continue(const char *cobol_src, int *is_valid_out);

// Zorse JCL IMPORT SYMLIST Parameter Auditor
int tsfi_zorse_validate_jcl_import(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INITIALIZE REPLACING Statement Auditor
int tsfi_zorse_validate_cobol_init_repl(const char *cobol_src, int *is_valid_out);

// Zorse JCL JCLLIB Parameter Auditor
int tsfi_zorse_validate_jcl_jcllib(const char *jcl_line, int *is_valid_out);

// z/VSE CICS DFHRESP Response Auditor
int tsfi_zorse_validate_vse_cics_dfhresp(const char *cobol_src, int *is_valid_out);

// z/VSE DL/I CBLTDLI Call Auditor
int tsfi_zorse_validate_vse_dli_call(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spooling Parameter Auditor
int tsfi_zorse_validate_vse_power_spool(const char *jcl_line, int *is_valid_out);

// z/VSE VSAM Record-Level Sharing (RLS) Lock Auditor
int tsfi_zorse_validate_vse_vsam_rls(const char *jcl_line, int *is_valid_out);

// Zorse and z/VSE Combined DAT-based LLM Query Interface
int tsfi_zorse_eval_dat_llm_query(const char *dat_bin_path, const char *prompt, char *response_out, size_t max_len);

// z/VSE VTAM Multi-Path Channel (MPC) Auditor
int tsfi_zorse_validate_vse_vtam_mpc(const char *jcl_line, int *is_valid_out);

// z/VSE Virtual Disk (FBA) Parameter Auditor
int tsfi_zorse_validate_vse_fba_vdisk(const char *jcl_line, int *is_valid_out);

// Zorse COBOL EVALUATE ALSO Statement Auditor
int tsfi_zorse_validate_cobol_evaluate_also(const char *cobol_src, int *is_valid_out);

// Zorse JCL VTAM PORT Parameter Auditor
int tsfi_zorse_validate_jcl_vtam_port(const char *jcl_line, int *is_valid_out);

// VSEn Vaesen System Registry APIs (Auncient Vaesen Tracking)
int tsfi_vsen_vaesen_register(const char *name, const char *type, int risk_level, const char *status);
int tsfi_vsen_vaesen_lookup(const char *name, char *type_out, int *risk_level_out, char *status_out, size_t max_len);
int tsfi_vsen_vaesen_audit_transaction(const char *cics_trans_id, const char *entity_name, int *is_allowed_out);

// VSEn Vaesen Sight Telemetry Tracker
int tsfi_vsen_vaesen_record_sight(const char *entity_name, const char *location, int fear_factor);
int tsfi_vsen_vaesen_get_aggregate_fear(const char *location, int *agg_fear_out);

// Zorse COBOL CALL USING Statement Auditor
int tsfi_zorse_validate_cobol_call_using(const char *cobol_src, int *is_valid_out);

// Zorse JCL SYMLIST Parameter Auditor
int tsfi_zorse_validate_jcl_symlist(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INSPECT REPLACING Statement Auditor
int tsfi_zorse_validate_cobol_inspect_repl(const char *cobol_src, int *is_valid_out);

// Zorse JCL DLURVTAM Parameter Auditor
int tsfi_zorse_validate_jcl_dlurvtam(const char *jcl_line, int *is_valid_out);

// Zorse COBOL EXIT PROGRAM Statement Auditor
int tsfi_zorse_validate_cobol_exit_program(const char *cobol_src, int *is_valid_out);

// Zorse JCL SYSOUT DEST Spool Router Auditor
int tsfi_zorse_validate_jcl_dest_router(const char *jcl_line, int *is_valid_out);

// Zorse COBOL STOP RUN Statement Auditor
int tsfi_zorse_validate_cobol_stop_run(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD DSID Parameter Auditor
int tsfi_zorse_validate_jcl_dsid(const char *jcl_line, int *is_valid_out);

// Zorse COBOL GOTO DEPENDING ON Statement Auditor
int tsfi_zorse_validate_cobol_goto_depending(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD DSNAME Parameter Auditor
int tsfi_zorse_validate_jcl_dsname(const char *jcl_line, int *is_valid_out);

// Zorse COBOL SORT Statement Auditor
int tsfi_zorse_validate_cobol_sort(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD PATH Parameter Auditor
int tsfi_zorse_validate_jcl_path(const char *jcl_line, int *is_valid_out);

// Zorse COBOL SEARCH ALL Statement Auditor
int tsfi_zorse_validate_cobol_search_all(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD GDG Generation Auditor
int tsfi_zorse_validate_jcl_gdg(const char *jcl_line, int *is_valid_out);

// Zorse COBOL READ NEXT Statement Auditor
int tsfi_zorse_validate_cobol_read_next(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD PATHMODE Parameter Auditor
int tsfi_zorse_validate_jcl_pathmode(const char *jcl_line, int *is_valid_out);

// Zorse COBOL WRITE FROM Statement Auditor
int tsfi_zorse_validate_cobol_write_from(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD GDG Version Auditor
int tsfi_zorse_validate_jcl_gdg_version(const char *jcl_line, int *is_valid_out);

// Zorse Ray-Tracer Volumetric Scatter Auditor
int tsfi_zorse_audit_volumetric_scatter(const char *image_b64, const char *model_name, char *report_out, size_t max_report_len);

// Zorse Ray-Tracer Subsurface Translucency Auditor
int tsfi_zorse_audit_subsurface_translucency(const char *image_b64, const char *model_name, char *report_out, size_t max_report_len);

// z/VSE Subsystem Support Services (Triple S Class) Auditor
int tsfi_zorse_validate_vse_triple_s(const char *jcl_line, int *is_valid_out);

// Zorse COBOL STRING WITH POINTER Statement Auditor
int tsfi_zorse_validate_cobol_string_pointer(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD PATHDISP Parameter Auditor
int tsfi_zorse_validate_jcl_pathdisp(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INITIALIZE WITH FILLER Statement Auditor
int tsfi_zorse_validate_cobol_init_filler(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD FILEDATA Parameter Auditor
int tsfi_zorse_validate_jcl_filedata(const char *jcl_line, int *is_valid_out);

// Zorse COBOL ENTRY USING Statement Auditor
int tsfi_zorse_validate_cobol_entry_using(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD PROTECT Parameter Auditor
int tsfi_zorse_validate_jcl_protect(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INSPECT TALLYING Statement Auditor
int tsfi_zorse_validate_cobol_inspect_tallying(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD SUBSYS Parameter Auditor
int tsfi_zorse_validate_jcl_subsys(const char *jcl_line, int *is_valid_out);

// Zorse COBOL INSPECT TALLYING REPLACING Statement Auditor
int tsfi_zorse_validate_cobol_inspect_tally_repl(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD FREE=CLOSE Parameter Auditor
int tsfi_zorse_validate_jcl_free_close(const char *jcl_line, int *is_valid_out);

// Zorse COBOL COPY OF Statement Auditor
int tsfi_zorse_validate_cobol_copy_of(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD ACCODE Parameter Auditor
int tsfi_zorse_validate_jcl_accode(const char *jcl_line, int *is_valid_out);

// Zorse COBOL XML GENERATE Statement Auditor
int tsfi_zorse_validate_cobol_xml_gen(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD DSNTYPE=LARGE Parameter Auditor
int tsfi_zorse_validate_jcl_dsntype_large(const char *jcl_line, int *is_valid_out);

// Zorse Stable Diffusion batch pipeline compliance auditor
int tsfi_zorse_validate_stable_diffusion(const char *jcl_line, const char *cobol_src, int *is_valid_out);

// Zorse Stable Diffusion Algol phase variance simulation
int tsfi_zorse_execute_stable_diffusion_algol(double base, double secret, double prime, double *noise_out);

// Zorse COBOL XML PARSE Statement Auditor
int tsfi_zorse_validate_cobol_xml_parse(const char *cobol_src, int *is_valid_out);

// Zorse JCL DD EROPT Parameter Auditor
int tsfi_zorse_validate_jcl_eropt(const char *jcl_line, int *is_valid_out);

// z/VSE POWER Queue Entry Submit Auditor
int tsfi_zorse_validate_vse_power_submit(const char *jcl_line, int *is_valid_out);

// JCL DD DSNAME Versioning Generation Auditor
int tsfi_zorse_validate_jcl_dsname_ver(const char *jcl_line, int *is_valid_out);

// Zorse relative to absolute GDG name resolver
int tsfi_zorse_resolve_gdg_relative_to_absolute(const char *gdg_base, int relative_gen, int current_gen, char *absolute_out, size_t max_len);

// z/VSE CICS RDO Compliance Auditor
int tsfi_zorse_validate_vse_cics_rdo(const char *rdo_script, int *is_valid_out);

// COBOL Dynamic Call Auditor
int tsfi_zorse_validate_cobol_dyn_call(const char *cobol_src, int *is_valid_out);

// Zorse Red DDL Strategy Language Auditor
int tsfi_zorse_validate_red_ddl(const char *ddl_src, int *is_valid_out);

// Zorse Black DML Data Model Auditor
int tsfi_zorse_validate_black_dml(const char *dml_src, int *is_valid_out);

// JCL EXEC PGM Parameter Override Auditor
int tsfi_zorse_validate_jcl_parm_override(const char *jcl_line, int *is_valid_out);

// JCL SYSOUT Routing Auditor
int tsfi_zorse_validate_jcl_sysout_route(const char *jcl_line, int *is_valid_out);

// z/VSE POWER Spool Queue Class Validator
int tsfi_zorse_validate_vse_power_class(const char *jcl_line, int *is_valid_out);

// COBOL Internal Sorting Auditor
int tsfi_zorse_validate_cobol_sort_key(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Validator
int tsfi_zorse_validate_vse_power_prtclass(const char *jcl_line, int *is_valid_out);

// COBOL File Merge Auditor
int tsfi_zorse_validate_cobol_merge_key(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Punch Class Validator
int tsfi_zorse_validate_vse_power_punclass(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Auditor
int tsfi_zorse_validate_cobol_generate(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Queue Hold Validator
int tsfi_zorse_validate_vse_power_hold(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Initiation Auditor
int tsfi_zorse_validate_cobol_initiate(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Queue Disposition Validator
int tsfi_zorse_validate_vse_power_disp(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Termination Auditor
int tsfi_zorse_validate_cobol_terminate(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Override Validator
int tsfi_zorse_validate_vse_power_prtclass_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Heading Auditor
int tsfi_zorse_validate_cobol_page_heading(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Copies Validator
int tsfi_zorse_validate_vse_power_copies(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Limit Auditor
int tsfi_zorse_validate_cobol_page_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Page Range Limit Validator
int tsfi_zorse_validate_vse_power_pages(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Heading Group Auditor
int tsfi_zorse_validate_cobol_heading_group(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Page Override Validator
int tsfi_zorse_validate_vse_power_pages_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Detail Group Auditor
int tsfi_zorse_validate_cobol_detail_group(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Printer Forms Validator
int tsfi_zorse_validate_vse_power_form(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Heading Auditor
int tsfi_zorse_validate_cobol_control_heading(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Line Limit Validator
int tsfi_zorse_validate_vse_power_lines(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Footing Auditor
int tsfi_zorse_validate_cobol_control_footing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Page Range List Validator
int tsfi_zorse_validate_vse_power_pages_list(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Footing Auditor
int tsfi_zorse_validate_cobol_page_footing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Page Count Limit Validator
int tsfi_zorse_validate_vse_power_pages_limit(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Report Heading Auditor
int tsfi_zorse_validate_cobol_report_heading(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Node Destination Validator
int tsfi_zorse_validate_vse_power_node(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Report Footing Auditor
int tsfi_zorse_validate_cobol_report_footing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Forms Overlay Validator
int tsfi_zorse_validate_vse_power_flash(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Footing Group Auditor
int tsfi_zorse_validate_cobol_cf_group(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Forms Flash Override Validator
int tsfi_zorse_validate_vse_power_flash_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Detail Group Line Spacing Auditor
int tsfi_zorse_validate_cobol_line_spacing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool forms flash overlay Status Validator
int tsfi_zorse_validate_vse_power_flash_status(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Next Page Spacing Auditor
int tsfi_zorse_validate_cobol_next_page(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Destination Node Status Validator
int tsfi_zorse_validate_vse_power_node_status(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Next Page Limit Auditor
int tsfi_zorse_validate_cobol_next_page_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Destination Node Override Validator
int tsfi_zorse_validate_vse_power_node_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Footing Spacing Auditor
int tsfi_zorse_validate_cobol_cf_spacing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Priority Validator
int tsfi_zorse_validate_vse_power_pri(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Footing Spacing Auditor
int tsfi_zorse_validate_cobol_pf_spacing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Priority Override Validator
int tsfi_zorse_validate_vse_power_pri_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Report Heading Spacing Auditor
int tsfi_zorse_validate_cobol_rh_spacing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Retention Validator
int tsfi_zorse_validate_vse_power_keep(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Group Auditor
int tsfi_zorse_validate_cobol_final_summary(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Retention Status Validator
int tsfi_zorse_validate_vse_power_keep_status(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Auditor
int tsfi_zorse_validate_cobol_final_spacing(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Override Validator
int tsfi_zorse_validate_vse_power_class_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Heading Spacing Limit Auditor
int tsfi_zorse_validate_cobol_ph_spacing_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Status Validator
int tsfi_zorse_validate_vse_power_class_status(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Page Footing Spacing Limit Auditor
int tsfi_zorse_validate_cobol_pf_spacing_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Retention Override Validator
int tsfi_zorse_validate_vse_power_keep_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Limit Auditor
int tsfi_zorse_validate_cobol_final_spacing_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Retention Status Override Validator
int tsfi_zorse_validate_vse_power_keep_status_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Limit Override Auditor
int tsfi_zorse_validate_cobol_final_spacing_limit_override(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Priority Status Override Validator
int tsfi_zorse_validate_vse_power_pri_status_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Limit Override Checks Auditor
int tsfi_zorse_validate_cobol_final_spacing_checks(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Priority Override List Validator
int tsfi_zorse_validate_vse_power_pri_override_list(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Limit Override Checks List Auditor
int tsfi_zorse_validate_cobol_final_spacing_list_checks(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Priority Status Override List Checks Validator
int tsfi_zorse_validate_vse_power_pri_status_checks(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Final Summary Spacing Limit Override Checks List Status Auditor
int tsfi_zorse_validate_cobol_final_spacing_status(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Override Status Validator
int tsfi_zorse_validate_vse_power_class_status_override(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Heading Spacing Limit Auditor
int tsfi_zorse_validate_cobol_ch_spacing_limit(const char *cobol_src, int *is_valid_out);

// z/VSE POWER Spool Print Class Override Status List Validator
int tsfi_zorse_validate_vse_power_class_status_override_list(const char *jcl_line, int *is_valid_out);

// COBOL Report Writer Control Heading Spacing Limit Override Auditor
int tsfi_zorse_validate_cobol_ch_spacing_limit_override(const char *cobol_src, int *is_valid_out);

#endif // TSFI_ZORSE_EVAL_H
