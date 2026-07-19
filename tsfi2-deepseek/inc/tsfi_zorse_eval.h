#ifndef TSFI_ZORSE_EVAL_H
#define TSFI_ZORSE_EVAL_H

// Zorse JCL Syntax Compliance Validator
int tsfi_zorse_validate_jcl(const char *jcl_string, int *is_valid_out, char *error_msg_out, int max_err_len);

// Zorse COBOL Division Structure Compliance Validator
int tsfi_zorse_validate_cobol(const char *cobol_string, int *is_valid_out);

#endif // TSFI_ZORSE_EVAL_H
