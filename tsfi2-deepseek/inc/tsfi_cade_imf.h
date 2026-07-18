#ifndef TSFI_CADE_IMF_H
#define TSFI_CADE_IMF_H

#include <stdint.h>
#include <stddef.h>

int tsfi_mf_cade_register_taxpayer(const char *ssn, double balance, int status, char *registry_out, int max_len);
int tsfi_mf_imf_process_transaction(const char *ssn, int transaction_code, double amount, double *balance_in_out, char *log_out, int max_len);
int tsfi_mf_cade_process_daily_batch(const char **ssns, const int *tcs, const double *amounts, int count, double *balances, char *batch_report, int max_len);
int tsfi_mf_imf_validate_transaction_code(int transaction_code, double amount, int *is_valid);
int tsfi_mf_imf_set_freeze_code(const char *ssn, char freeze_code, char *account_state, int max_len);
int tsfi_mf_imf_is_frozen(const char *account_state, int *is_frozen);
int tsfi_mf_cade_lookup_taxpayer(const char *ssn, const char *registry_pool, double *balance_out, int *status_out);
int tsfi_mf_imf_encode_cycle_code(int year, int week, int day, char *cycle_out, int max_len);
int tsfi_mf_imf_decode_cycle_code(const char *cycle_code, int *year, int *week, int *day);
int tsfi_mf_cade_update_taxpayer_status(char *registry_entry, int new_status);
int tsfi_mf_imf_parse_dln(const char *dln, int *site_code, int *tax_class, int *doc_code, int *julian_date, int *serial_num);
int tsfi_mf_cade_adjust_balance(char *registry_entry, double offset_amount);
int tsfi_mf_imf_generate_dln(int site_code, int tax_class, int doc_code, int julian_date, int serial_num, char *dln_out, int max_len);
int tsfi_mf_imf_get_tax_class_name(int tax_class, char *name_out, int max_len);
int tsfi_mf_imf_get_transaction_sign(int transaction_code, int *sign_out);
int tsfi_mf_imf_reconcile_balance(double initial_balance, const int *tcs, const double *amounts, int count, double final_balance, int *is_reconciled);
int tsfi_mf_imf_validate_dln(const char *dln, int *is_valid);
int tsfi_mf_cade_format_batch_summary(int processed, int errors, double total_debit, double total_credit, char *out_buf, int max_len);
int tsfi_mf_imf_get_doc_code_name(int doc_code, char *name_out, int max_len);
int tsfi_mf_cade_export_registry(const char *registry_pool, char *export_buf, int max_len);
int tsfi_mf_imf_validate_doc_code(int doc_code, int *is_valid);
int tsfi_mf_cade_compact_registry(char *registry_pool);
int tsfi_mf_imf_get_doc_code_by_name(const char *name, int *doc_code_out);
int tsfi_mf_cade_locate_record_offset(const char *ssn, const char *registry_pool, int *offset_out);
int tsfi_mf_cade_get_status_name(int status_code, char *name_out, int max_len);

int tsfi_mf_ddl_set_mode(int mode);
int tsfi_mf_ddl_execute(const char *statement, char *output, size_t out_len);
int tsfi_mf_dml_set_mode(int mode);
int tsfi_mf_dml_execute(const char *statement, char *output, size_t out_len);
int tsfi_mf_cade_verify_address(const char *addr_a, const char *addr_b, int *is_match);
int tsfi_mf_imf_get_cycle_week(const char *cycle_code, int *week_out);

int tsfi_mf_imf_verify_document_form(int doc_code, int target_code, int *result);
int tsfi_mf_cade_check_status_mask(int status_code, unsigned int mask, int *is_valid);
int tsfi_mf_cade_route_partition(const char *ssn, int *partition_out);
int tsfi_mf_imf_verify_ssn_check_digit(const char *ssn, int *is_valid);
int tsfi_mf_imf_evaluate_audit_discrepancy(double reported_income, double documented_income, int missing_schedules, int *audit_flag);

#endif // TSFI_CADE_IMF_H
