#ifndef TSFI_CADE_IMF_H
#define TSFI_CADE_IMF_H

#include <stdint.h>

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

#endif // TSFI_CADE_IMF_H
