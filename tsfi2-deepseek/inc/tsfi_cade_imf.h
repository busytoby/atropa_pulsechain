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
int tsfi_mf_imf_schedule_refund(const char *cycle_code, int transaction_code, char *schedule_date_out, int max_len);
int tsfi_mf_imf_apply_liability_offset(double refund_amt, double liability_amt, double *net_refund_out, double *remaining_liability_out);
int tsfi_mf_imf_verify_filing_status(int filing_status, int doc_code, int *is_allowed);
int tsfi_mf_cade_classify_batch_error(int error_code, char *severity_out, int max_len);
int tsfi_mf_imf_verify_agi_calculation(double gross_income, double adjustments, double reported_agi, int *is_match);
int tsfi_mf_imf_validate_deductions(int filing_status, double claimed_deductions, int use_itemized, int *is_valid);
int tsfi_mf_cade_check_refund_hold(int identity_verified, int address_verified, int *has_hold);
int tsfi_mf_imf_verify_signatures(int filing_status, int has_taxpayer_sig, int has_spouse_sig, int *is_valid);
int tsfi_mf_imf_check_dependent_duplicates(const char *primary_ssn, const char *secondary_ssn, const char **dep_ssns, int dep_count, int *has_duplicates);
int tsfi_mf_imf_verify_filing_deadline(int julian_date_filed, int has_approved_extension, int *is_timely);
int tsfi_mf_cade_route_refund_disbursement(const char *routing_number, const char *account_number, int *method_out);
int tsfi_mf_imf_verify_refund_threshold(double refund_amount, double limit, int *requires_review);
int tsfi_mf_cade_verify_prior_year_agi(double input_agi, double actual_prior_agi, int *is_match);
int tsfi_mf_imf_classify_agi_bracket(double agi, int *bracket_out);
int tsfi_mf_cade_verify_designee_record(int has_designee, const char *designee_pin, int *is_valid);

#endif // TSFI_CADE_IMF_H
