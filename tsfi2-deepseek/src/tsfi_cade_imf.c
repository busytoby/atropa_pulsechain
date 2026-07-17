#include "tsfi_cade_imf.h"
#include <stdio.h>
#include <string.h>

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

int tsfi_mf_imf_is_corporate_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 20) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_under_audit(int status_code, int *is_audit) {
    if (!is_audit) return -1;

    *is_audit = 0;
    if (status_code == 2) {
        *is_audit = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_individual_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 10) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_active(int status_code, int *is_active) {
    if (!is_active) return -1;

    *is_active = 0;
    if (status_code == 1) {
        *is_active = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_fica_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 30) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_suspended(int status_code, int *is_suspended) {
    if (!is_suspended) return -1;

    *is_suspended = 0;
    if (status_code == 3) {
        *is_suspended = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_excise_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 40) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_audit_pending(int status_code, int *is_pending) {
    if (!is_pending) return -1;

    *is_pending = 0;
    if (status_code == 4) {
        *is_pending = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_misc_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 50) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_unknown_status(int status_code, int *is_unknown) {
    if (!is_unknown) return -1;

    *is_unknown = 0;
    if (status_code < 1 || status_code > 4) {
        *is_unknown = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_partnership_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 15) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_are_both_active(int status_a, int status_b, int *both_active) {
    if (!both_active) return -1;

    *both_active = 0;
    if (status_a == 1 && status_b == 1) {
        *both_active = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_estate_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 25) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_audit_or_pending(int status_code, int *is_audit_or_pending) {
    if (!is_audit_or_pending) return -1;

    *is_audit_or_pending = 0;
    if (status_code == 2 || status_code == 4) {
        *is_audit_or_pending = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_gift_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 26) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_active_or_suspended(int status_code, int *is_active_or_suspended) {
    if (!is_active_or_suspended) return -1;

    *is_active_or_suspended = 0;
    if (status_code == 1 || status_code == 3) {
        *is_active_or_suspended = 1;
    }
    return 0;
}

int tsfi_mf_imf_is_fiduciary_form(int doc_code, int *result) {
    if (!result) return -1;

    *result = 0;
    if (doc_code == 11) {
        *result = 1;
    }
    return 0;
}

int tsfi_mf_cade_is_active_or_under_audit(int status_code, int *is_active_or_audit) {
    if (!is_active_or_audit) return -1;

    *is_active_or_audit = 0;
    if (status_code == 1 || status_code == 2) {
        *is_active_or_audit = 1;
    }
    return 0;
}
