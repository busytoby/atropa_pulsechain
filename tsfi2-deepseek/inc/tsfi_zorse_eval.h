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

#endif // TSFI_ZORSE_EVAL_H
