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

#endif // TSFI_ZORSE_EVAL_H
