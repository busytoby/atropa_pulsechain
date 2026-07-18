#include "tsfi_cade_imf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

int tsfi_mf_cade_register_taxpayer(const char *ssn, double balance, int status, char *registry_out, int max_len) {
    if (!ssn || !registry_out || max_len <= 0) return -1;

    snprintf(registry_out, max_len, "SSN:%s|BAL:%.2f|STATUS:%d", ssn, balance, status);
    return 0;
}

int tsfi_mf_imf_process_transaction(const char *ssn, int transaction_code, double amount, double *balance_in_out, char *log_out, int max_len) {
    if (!ssn || !balance_in_out || !log_out || max_len <= 0) return -1;

    switch (transaction_code) {
        case 150:
            *balance_in_out += amount;
            snprintf(log_out, max_len, "TC150: Tax Return Filed for SSN %s. Assessment: +%.2f", ssn, amount);
            break;
        case 846:
            *balance_in_out -= amount;
            snprintf(log_out, max_len, "TC846: Refund Issued for SSN %s. Amount: -%.2f", ssn, amount);
            break;
        case 290:
            *balance_in_out += amount;
            snprintf(log_out, max_len, "TC290: Additional Tax Assessment for SSN %s. Amount: +%.2f", ssn, amount);
            break;
        default:
            snprintf(log_out, max_len, "TC%d: Unknown IMF Transaction Code for SSN %s", transaction_code, ssn);
            return -2;
    }
    return 0;
}

int tsfi_mf_cade_process_daily_batch(const char **ssns, const int *tcs, const double *amounts, int count, double *balances, char *batch_report, int max_len) {
    if (!ssns || !tcs || !amounts || !balances || !batch_report || max_len <= 0) return -1;

    double total_assessments = 0.0;
    double total_refunds = 0.0;
    int success_count = 0;
    char temp_log[128];

    for (int i = 0; i < count; i++) {
        int res = tsfi_mf_imf_process_transaction(ssns[i], tcs[i], amounts[i], &balances[i], temp_log, sizeof(temp_log));
        if (res == 0) {
            success_count++;
            if (tcs[i] == 150 || tcs[i] == 290) {
                total_assessments += amounts[i];
            } else if (tcs[i] == 846) {
                total_refunds += amounts[i];
            }
        }
    }

    snprintf(batch_report, max_len, "CADE DAILY BATCH: PROCESSED %d/%d | ASSESSMENTS: %.2f | REFUNDS: %.2f",
             success_count, count, total_assessments, total_refunds);
    return 0;
}

int tsfi_mf_imf_validate_transaction_code(int transaction_code, double amount, int *is_valid) {
    if (!is_valid) return -1;

    *is_valid = 0;
    if (amount >= 0.0 && (transaction_code == 150 || transaction_code == 846 || transaction_code == 290)) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_imf_set_freeze_code(const char *ssn, char freeze_code, char *account_state, int max_len) {
    if (!ssn || !account_state || max_len <= 0) return -1;

    snprintf(account_state, max_len, "SSN:%s|FREEZE:%c", ssn, freeze_code);
    return 0;
}

int tsfi_mf_imf_is_frozen(const char *account_state, int *is_frozen) {
    if (!account_state || !is_frozen) return -1;

    *is_frozen = 0;
    const char *freeze_ptr = strstr(account_state, "FREEZE:");
    if (freeze_ptr) {
        char f_char = freeze_ptr[7];
        if (f_char != '\0' && f_char != ' ' && f_char != '0') {
            *is_frozen = 1;
        }
    }
    return 0;
}

int tsfi_mf_cade_lookup_taxpayer(const char *ssn, const char *registry_pool, double *balance_out, int *status_out) {
    if (!ssn || !registry_pool || !balance_out || !status_out) return -1;

    const char *ssn_match = strstr(registry_pool, ssn);
    if (ssn_match) {
        const char *bal_ptr = strstr(ssn_match, "BAL:");
        const char *status_ptr = strstr(ssn_match, "STATUS:");
        if (bal_ptr && status_ptr) {
            if (sscanf(bal_ptr, "BAL:%lf", balance_out) == 1 &&
                sscanf(status_ptr, "STATUS:%d", status_out) == 1) {
                return 0;
            }
        }
    }
    return -2;
}

int tsfi_mf_imf_encode_cycle_code(int year, int week, int day, char *cycle_out, int max_len) {
    if (!cycle_out || max_len < 9) return -1;

    snprintf(cycle_out, max_len, "%04d%02d%02d", year, week, day);
    return 0;
}

int tsfi_mf_imf_decode_cycle_code(const char *cycle_code, int *year, int *week, int *day) {
    if (!cycle_code || !year || !week || !day) return -1;

    if (sscanf(cycle_code, "%4d%2d%2d", year, week, day) == 3) {
        return 0;
    }
    return -2;
}

int tsfi_mf_cade_update_taxpayer_status(char *registry_entry, int new_status) {
    if (!registry_entry) return -1;

    char *status_ptr = strstr(registry_entry, "STATUS:");
    if (status_ptr) {
        snprintf(status_ptr, 16, "STATUS:%d", new_status);
        return 0;
    }
    return -2;
}

int tsfi_mf_imf_parse_dln(const char *dln, int *site_code, int *tax_class, int *doc_code, int *julian_date, int *serial_num) {
    if (!dln || !site_code || !tax_class || !doc_code || !julian_date || !serial_num) return -1;

    if (sscanf(dln, "%2d%1d%2d%3d%6d", site_code, tax_class, doc_code, julian_date, serial_num) == 5) {
        return 0;
    }
    return -2;
}

int tsfi_mf_cade_adjust_balance(char *registry_entry, double offset_amount) {
    if (!registry_entry) return -1;

    char *bal_ptr = strstr(registry_entry, "BAL:");
    if (bal_ptr) {
        double current_bal = 0.0;
        if (sscanf(bal_ptr, "BAL:%lf", &current_bal) == 1) {
            current_bal += offset_amount;
            char temp_buf[64];
            snprintf(temp_buf, sizeof(temp_buf), "BAL:%.2f", current_bal);
            char *bar_ptr = strchr(bal_ptr, '|');
            if (bar_ptr) {
                char suffix[64];
                strcpy(suffix, bar_ptr);
                int prefix_len = bal_ptr - registry_entry;
                char prefix[128];
                memcpy(prefix, registry_entry, prefix_len);
                prefix[prefix_len] = '\0';
                sprintf(registry_entry, "%s%s%s", prefix, temp_buf, suffix);
            } else {
                int prefix_len = bal_ptr - registry_entry;
                char prefix[128];
                memcpy(prefix, registry_entry, prefix_len);
                prefix[prefix_len] = '\0';
                sprintf(registry_entry, "%s%s", prefix, temp_buf);
            }
            return 0;
        }
    }
    return -2;
}

int tsfi_mf_imf_generate_dln(int site_code, int tax_class, int doc_code, int julian_date, int serial_num, char *dln_out, int max_len) {
    if (!dln_out || max_len < 15) return -1;

    snprintf(dln_out, max_len, "%02d%01d%02d%03d%06d", site_code, tax_class, doc_code, julian_date, serial_num);
    return 0;
}

int tsfi_mf_imf_get_tax_class_name(int tax_class, char *name_out, int max_len) {
    if (!name_out || max_len <= 0) return -1;

    switch (tax_class) {
        case 1:
            snprintf(name_out, max_len, "Individual Income");
            break;
        case 2:
            snprintf(name_out, max_len, "FICA Withholding");
            break;
        case 3:
            snprintf(name_out, max_len, "Corporation Tax");
            break;
        case 4:
            snprintf(name_out, max_len, "Excise Tax");
            break;
        default:
            snprintf(name_out, max_len, "Other/Unknown");
            break;
    }
    return 0;
}

int tsfi_mf_imf_get_transaction_sign(int transaction_code, int *sign_out) {
    if (!sign_out) return -1;

    switch (transaction_code) {
        case 150:
        case 290:
            *sign_out = 1;
            break;
        case 846:
        case 610:
            *sign_out = -1;
            break;
        default:
            *sign_out = 0;
            return -2;
    }
    return 0;
}

int tsfi_mf_imf_reconcile_balance(double initial_balance, const int *tcs, const double *amounts, int count, double final_balance, int *is_reconciled) {
    if (!tcs || !amounts || !is_reconciled) return -1;

    double running_bal = initial_balance;
    for (int i = 0; i < count; i++) {
        int sign = 0;
        int sign_res = tsfi_mf_imf_get_transaction_sign(tcs[i], &sign);
        if (sign_res == 0) {
            running_bal += (sign * amounts[i]);
        }
    }

    *is_reconciled = 0;
    double diff = running_bal - final_balance;
    if (diff < 0.0) diff = -diff;
    if (diff < 0.001) {
        *is_reconciled = 1;
    }
    return 0;
}

int tsfi_mf_imf_validate_dln(const char *dln, int *is_valid) {
    if (!dln || !is_valid) return -1;

    *is_valid = 0;
    int len = strlen(dln);
    if (len != 14) return 0;

    for (int i = 0; i < 14; i++) {
        if (dln[i] < '0' || dln[i] > '9') return 0;
    }

    int site = 0, tax_class = 0, doc_code = 0, julian = 0, serial = 0;
    if (sscanf(dln, "%2d%1d%2d%3d%6d", &site, &tax_class, &doc_code, &julian, &serial) == 5) {
        if (site > 0 && tax_class > 0 && doc_code >= 0 && julian > 0 && julian <= 366 && serial >= 0) {
            *is_valid = 1;
        }
    }
    return 0;
}

int tsfi_mf_cade_format_batch_summary(int processed, int errors, double total_debit, double total_credit, char *out_buf, int max_len) {
    if (!out_buf || max_len <= 0) return -1;

    snprintf(out_buf, max_len, "PROC:%06d|ERR:%04d|DEBIT:%.2f|CREDIT:%.2f", processed, errors, total_debit, total_credit);
    return 0;
}

int tsfi_mf_imf_get_doc_code_name(int doc_code, char *name_out, int max_len) {
    if (!name_out || max_len <= 0) return -1;

    switch (doc_code) {
        case 10:
            snprintf(name_out, max_len, "Form 1040");
            break;
        case 20:
            snprintf(name_out, max_len, "Form 1120");
            break;
        case 30:
            snprintf(name_out, max_len, "Form 941");
            break;
        default:
            snprintf(name_out, max_len, "Form Misc");
            break;
    }
    return 0;
}

int tsfi_mf_cade_export_registry(const char *registry_pool, char *export_buf, int max_len) {
    if (!registry_pool || !export_buf || max_len <= 0) return -1;

    int copy_len = strlen(registry_pool);
    if (copy_len >= max_len) {
        copy_len = max_len - 1;
    }
    memcpy(export_buf, registry_pool, copy_len);
    export_buf[copy_len] = '\0';
    return 0;
}

int tsfi_mf_imf_validate_doc_code(int doc_code, int *is_valid) {
    if (!is_valid) return -1;

    *is_valid = 0;
    if (doc_code == 10 || doc_code == 20 || doc_code == 30) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_cade_compact_registry(char *registry_pool) {
    if (!registry_pool) return -1;

    int read_idx = 0;
    int write_idx = 0;
    while (registry_pool[read_idx] != '\0') {
        if (registry_pool[read_idx] != ' ') {
            registry_pool[write_idx++] = registry_pool[read_idx];
        }
        read_idx++;
    }
    registry_pool[write_idx] = '\0';
    return 0;
}

int tsfi_mf_imf_get_doc_code_by_name(const char *name, int *doc_code_out) {
    if (!name || !doc_code_out) return -1;

    if (strcmp(name, "Form 1040") == 0) {
        *doc_code_out = 10;
    } else if (strcmp(name, "Form 1120") == 0) {
        *doc_code_out = 20;
    } else if (strcmp(name, "Form 941") == 0) {
        *doc_code_out = 30;
    } else {
        *doc_code_out = 0;
        return -2;
    }
    return 0;
}

int tsfi_mf_cade_locate_record_offset(const char *ssn, const char *registry_pool, int *offset_out) {
    if (!ssn || !registry_pool || !offset_out) return -1;

    const char *ssn_match = strstr(registry_pool, ssn);
    if (ssn_match) {
        *offset_out = ssn_match - registry_pool;
        return 0;
    }
    *offset_out = -1;
    return -2;
}

int tsfi_mf_imf_is_excise_or_misc_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code >= 40) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_get_status_name(int status_code, char *name_out, int max_len) {
    if (!name_out || max_len <= 0) return -1;

    switch (status_code) {
        case 1:
            snprintf(name_out, max_len, "Active");
            break;
        case 2:
            snprintf(name_out, max_len, "Under Audit");
            break;
        case 3:
            snprintf(name_out, max_len, "Pending Collection");
            break;
        default:
            snprintf(name_out, max_len, "Unknown Status");
            break;
    }
    return 0;
}

int tsfi_mf_imf_verify_document_form(int doc_code, int target_code, int *result) {
    if (!result) return -1;
    *result = (doc_code == target_code) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_check_status_mask(int status_code, unsigned int mask, int *is_valid) {
    if (!is_valid) return -1;
    if (status_code >= 1 && status_code <= 4) {
        *is_valid = (mask & (1U << (status_code - 1))) ? 1 : 0;
    } else {
        *is_valid = 0;
    }
    return 0;
}

static int g_ddl_mode = 0; // 0 = COBOL, 1 = Thunks
static int g_dml_mode = 0; // 0 = ALGOL 61, 1 = Thunks

int tsfi_mf_ddl_set_mode(int mode) {
    if (mode < 0 || mode > 1) return -1;
    g_ddl_mode = mode;
    return 0;
}

int tsfi_mf_ddl_execute(const char *statement, char *output, size_t out_len) {
    if (!statement || !output || out_len == 0) return -1;
    if (g_ddl_mode == 0) {
        snprintf(output, out_len, "COBOL DDL Shader Compiler: Executed schema generation for '%s'", statement);
    } else {
        snprintf(output, out_len, "Thunk DDL Executor: Routed transaction via dynamic layout thunks");
    }
    return 0;
}

int tsfi_mf_dml_set_mode(int mode) {
    if (mode < 0 || mode > 1) return -1;
    g_dml_mode = mode;
    return 0;
}

int tsfi_mf_dml_execute(const char *statement, char *output, size_t out_len) {
    if (!statement || !output || out_len == 0) return -1;
    if (g_dml_mode == 0) {
        snprintf(output, out_len, "ALGOL 61 DML DNA Compiler: Mutated logic states for '%s'", statement);
    } else {
        snprintf(output, out_len, "Thunk DML Executor: Routed transaction via dynamic execution thunks");
    }
    return 0;
}

int tsfi_mf_cade_verify_address(const char *addr_a, const char *addr_b, int *is_match) {
    if (!addr_a || !addr_b || !is_match) return -1;
    *is_match = (strcasecmp(addr_a, addr_b) == 0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_get_cycle_week(const char *cycle_code, int *week_out) {
    if (!cycle_code || !week_out) return -1;
    if (strlen(cycle_code) < 6) return -1;
    char week_str[3] = { cycle_code[4], cycle_code[5], '\0' };
    *week_out = atoi(week_str);
    return 0;
}

int tsfi_mf_cade_route_partition(const char *ssn, int *partition_out) {
    if (!ssn || !partition_out) return -1;
    if (strlen(ssn) != 11) return -1;
    if (ssn[3] != '-' || ssn[6] != '-') return -1;
    char digit_str[3] = { ssn[9], ssn[10], '\0' };
    int val = atoi(digit_str);
    *partition_out = val / 10;
    return 0;
}

int tsfi_mf_imf_verify_ssn_check_digit(const char *ssn, int *is_valid) {
    if (!ssn || !is_valid) return -1;
    if (strlen(ssn) != 11) return -1;
    if (ssn[3] != '-' || ssn[6] != '-') return -1;
    int sum = 0;
    for (int i = 0; i < 11; i++) {
        if (i != 3 && i != 6) {
            sum = (sum * 10 + (ssn[i] - '0')) % 97;
        }
    }
    *is_valid = (sum == 0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_evaluate_audit_discrepancy(double reported_income, double documented_income, int missing_schedules, int *audit_flag) {
    if (!audit_flag) return -1;
    double difference = documented_income - reported_income;
    if (difference < 0) {
        difference = -difference;
    }
    if ((reported_income > 0 && (difference / reported_income) > 0.10) || missing_schedules > 0) {
        *audit_flag = 1;
    } else {
        *audit_flag = 0;
    }
    return 0;
}

int tsfi_mf_imf_schedule_refund(const char *cycle_code, int transaction_code, char *schedule_date_out, int max_len) {
    if (!cycle_code || !schedule_date_out || max_len <= 0) return -1;
    if (transaction_code != 846) {
        snprintf(schedule_date_out, max_len, "N/A");
        return 0;
    }
    int year = 0, week = 0, day = 0;
    if (tsfi_mf_imf_decode_cycle_code(cycle_code, &year, &week, &day) != 0) {
        return -1;
    }
    int refund_week = week + 1;
    snprintf(schedule_date_out, max_len, "%d-W%02d", year, refund_week);
    return 0;
}

int tsfi_mf_imf_apply_liability_offset(double refund_amt, double liability_amt, double *net_refund_out, double *remaining_liability_out) {
    if (!net_refund_out || !remaining_liability_out) return -1;
    if (refund_amt < 0 || liability_amt < 0) return -1;
    if (refund_amt >= liability_amt) {
        *net_refund_out = refund_amt - liability_amt;
        *remaining_liability_out = 0.0;
    } else {
        *net_refund_out = 0.0;
        *remaining_liability_out = liability_amt - refund_amt;
    }
    return 0;
}

int tsfi_mf_imf_verify_filing_status(int filing_status, int doc_code, int *is_allowed) {
    if (!is_allowed) return -1;
    if (filing_status == 3 && doc_code == 88) {
        *is_allowed = 0;
    } else {
        *is_allowed = 1;
    }
    return 0;
}

int tsfi_mf_cade_classify_batch_error(int error_code, char *severity_out, int max_len) {
    if (!severity_out || max_len <= 0) return -1;
    if (error_code >= 500) {
        snprintf(severity_out, max_len, "CRITICAL");
    } else if (error_code >= 400) {
        snprintf(severity_out, max_len, "REJECTED");
    } else if (error_code >= 300) {
        snprintf(severity_out, max_len, "WARNING");
    } else {
        snprintf(severity_out, max_len, "INFO");
    }
    return 0;
}

int tsfi_mf_imf_verify_agi_calculation(double gross_income, double adjustments, double reported_agi, int *is_match) {
    if (!is_match) return -1;
    double expected_agi = gross_income - adjustments;
    double diff = expected_agi - reported_agi;
    if (diff < 0) diff = -diff;
    *is_match = (diff < 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_validate_deductions(int filing_status, double claimed_deductions, int use_itemized, int *is_valid) {
    if (!is_valid) return -1;
    if (use_itemized) {
        *is_valid = 1;
        return 0;
    }
    double standard_deduction = 12000.00;
    if (filing_status == 2) {
        standard_deduction = 24000.00;
    }
    *is_valid = (claimed_deductions == standard_deduction) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_check_refund_hold(int identity_verified, int address_verified, int *has_hold) {
    if (!has_hold) return -1;
    *has_hold = (!identity_verified || !address_verified) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_signatures(int filing_status, int has_taxpayer_sig, int has_spouse_sig, int *is_valid) {
    if (!is_valid) return -1;
    if (filing_status == 2) {
        *is_valid = (has_taxpayer_sig && has_spouse_sig) ? 1 : 0;
    } else {
        *is_valid = (has_taxpayer_sig) ? 1 : 0;
    }
    return 0;
}

int tsfi_mf_imf_check_dependent_duplicates(const char *primary_ssn, const char *secondary_ssn, const char **dep_ssns, int dep_count, int *has_duplicates) {
    if (!primary_ssn || !has_duplicates) return -1;
    *has_duplicates = 0;
    for (int i = 0; i < dep_count; i++) {
        if (!dep_ssns[i]) continue;
        if (strcmp(dep_ssns[i], primary_ssn) == 0) {
            *has_duplicates = 1;
            return 0;
        }
        if (secondary_ssn && strcmp(dep_ssns[i], secondary_ssn) == 0) {
            *has_duplicates = 1;
            return 0;
        }
        for (int j = i + 1; j < dep_count; j++) {
            if (dep_ssns[j] && strcmp(dep_ssns[i], dep_ssns[j]) == 0) {
                *has_duplicates = 1;
                return 0;
            }
        }
    }
    return 0;
}

int tsfi_mf_imf_verify_filing_deadline(int julian_date_filed, int has_approved_extension, int *is_timely) {
    if (!is_timely) return -1;
    int deadline = has_approved_extension ? 288 : 105;
    *is_timely = (julian_date_filed <= deadline) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_route_refund_disbursement(const char *routing_number, const char *account_number, int *method_out) {
    if (!method_out) return -1;
    if (!routing_number || !account_number || strlen(account_number) == 0) {
        *method_out = 2;
        return 0;
    }
    if (strlen(routing_number) != 9) {
        *method_out = 2;
        return 0;
    }
    for (int i = 0; i < 9; i++) {
        if (routing_number[i] < '0' || routing_number[i] > '9') {
            *method_out = 2;
            return 0;
        }
    }
    *method_out = 1;
    return 0;
}

int tsfi_mf_imf_verify_refund_threshold(double refund_amount, double limit, int *requires_review) {
    if (!requires_review) return -1;
    *requires_review = (refund_amount > limit) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_prior_year_agi(double input_agi, double actual_prior_agi, int *is_match) {
    if (!is_match) return -1;
    double diff = input_agi - actual_prior_agi;
    if (diff < 0) diff = -diff;
    *is_match = (diff < 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_classify_agi_bracket(double agi, int *bracket_out) {
    if (!bracket_out) return -1;
    if (agi < 30000.00) {
        *bracket_out = 1;
    } else if (agi < 100000.00) {
        *bracket_out = 2;
    } else if (agi < 500000.00) {
        *bracket_out = 3;
    } else {
        *bracket_out = 4;
    }
    return 0;
}

int tsfi_mf_cade_verify_designee_record(int has_designee, const char *designee_pin, int *is_valid) {
    if (!is_valid) return -1;
    if (!has_designee) {
        *is_valid = 1;
        return 0;
    }
    if (!designee_pin || strlen(designee_pin) != 5) {
        *is_valid = 0;
        return 0;
    }
    for (int i = 0; i < 5; i++) {
        if (designee_pin[i] < '0' || designee_pin[i] > '9') {
            *is_valid = 0;
            return 0;
        }
    }
    *is_valid = 1;
    return 0;
}


