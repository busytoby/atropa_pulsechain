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
