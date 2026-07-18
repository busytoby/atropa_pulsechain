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

static char g_db_registry[4096] = {0};

int tsfi_mf_ddl_execute(const char *statement, char *output, size_t out_len) {
    if (!statement || !output || out_len == 0) return -1;
    if (strncasecmp(statement, "CREATE TABLE", 12) == 0) {
        g_db_registry[0] = '\0';
        if (g_ddl_mode == 0) {
            snprintf(output, out_len, "COBOL DDL: Created table. Registry reset.");
        } else {
            snprintf(output, out_len, "Thunk DDL: Initialized partition layout.");
        }
    } else {
        snprintf(output, out_len, "DDL: Executed '%s'", statement);
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
    if (strncasecmp(statement, "INSERT INTO taxpayers", 21) == 0) {
        char ssn[16] = {0};
        double balance = 0.0;
        int status = 0;
        const char *p = strchr(statement, '\'');
        if (p) {
            int i = 0;
            p++;
            while (*p != '\'' && *p != '\0' && i < 15) {
                ssn[i++] = *p++;
            }
            if (*p == '\'') {
                p++;
                if (*p == ',') p++;
                balance = atof(p);
                p = strchr(p, ',');
                if (p) {
                    p++;
                    status = atoi(p);
                }
            }
        }
        if (strlen(ssn) == 11) {
            char entry[128] = {0};
            int reg_res = tsfi_mf_cade_register_taxpayer(ssn, balance, status, entry, sizeof(entry));
            if (reg_res == 0) {
                strncat(g_db_registry, entry, sizeof(g_db_registry) - strlen(g_db_registry) - 1);
                strncat(g_db_registry, "\n", sizeof(g_db_registry) - strlen(g_db_registry) - 1);
                if (g_dml_mode == 0) {
                    snprintf(output, out_len, "ALGOL 61 DML: Inserted record for %s", ssn);
                } else {
                    snprintf(output, out_len, "Thunk DML: Inserted record for %s", ssn);
                }
            } else {
                snprintf(output, out_len, "DML Error: Register failed");
            }
        } else {
            snprintf(output, out_len, "DML Error: Parse failed");
        }
    } else if (strncasecmp(statement, "SELECT balance FROM taxpayers", 29) == 0) {
        char ssn[16] = {0};
        const char *p = strchr(statement, '\'');
        if (p) {
            int i = 0;
            p++;
            while (*p != '\'' && *p != '\0' && i < 15) {
                ssn[i++] = *p++;
            }
        }
        double balance = 0.0;
        int status = 0;
        int lookup_res = tsfi_mf_cade_lookup_taxpayer(ssn, g_db_registry, &balance, &status);
        if (lookup_res == 0) {
            snprintf(output, out_len, "BALANCE:%.2f", balance);
        } else {
            snprintf(output, out_len, "DML Error: Taxpayer not found");
        }
    } else {
        snprintf(output, out_len, "DML: Executed '%s'", statement);
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

int tsfi_mf_cade_verify_filing_method(int method_code, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (method_code >= 1 && method_code <= 3) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_check_first_time_filer(const char *ssn, const char *registry_pool, int *is_first_time) {
    if (!ssn || !is_first_time) return -1;
    if (!registry_pool || strstr(registry_pool, ssn) == NULL) {
        *is_first_time = 1;
    } else {
        *is_first_time = 0;
    }
    return 0;
}

int tsfi_mf_cade_verify_phone_area_code(const char *phone, const char *state_code, int *is_match) {
    if (!phone || !state_code || !is_match) return -1;
    *is_match = 0;
    char area[4] = {0};
    int count = 0;
    for (int i = 0; phone[i] != '\0' && count < 3; i++) {
        if (phone[i] >= '0' && phone[i] <= '9') {
            area[count++] = phone[i];
        }
    }
    if (count < 3) return 0;
    if (strcmp(state_code, "CA") == 0) {
        if (strcmp(area, "213") == 0 || strcmp(area, "310") == 0 || strcmp(area, "415") == 0 || strcmp(area, "650") == 0) {
            *is_match = 1;
        }
    } else if (strcmp(state_code, "NY") == 0) {
        if (strcmp(area, "212") == 0 || strcmp(area, "718") == 0 || strcmp(area, "917") == 0) {
            *is_match = 1;
        }
    } else {
        *is_match = 1;
    }
    return 0;
}

int tsfi_mf_imf_verify_dependent_age(int birth_year, int tax_year, int *is_eligible) {
    if (!is_eligible) return -1;
    int age = tax_year - birth_year;
    *is_eligible = (age >= 0 && age <= 16) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_zip_state(const char *zip, const char *state_code, int *is_match) {
    if (!zip || !state_code || !is_match) return -1;
    *is_match = 0;
    if (strlen(zip) < 2) return 0;
    char prefix[3] = {zip[0], zip[1], '\0'};
    int pref = atoi(prefix);
    if (strcmp(state_code, "CA") == 0) {
        if (pref >= 90 && pref <= 96) *is_match = 1;
    } else if (strcmp(state_code, "NY") == 0) {
        if (pref >= 9 && pref <= 14) *is_match = 1;
    } else if (strcmp(state_code, "TX") == 0) {
        if (pref >= 75 && pref <= 79) *is_match = 1;
    } else {
        *is_match = 1;
    }
    return 0;
}

int tsfi_mf_imf_verify_eitc_qualifying_child(const char *relationship, int residency_days, int *is_eligible) {
    if (!relationship || !is_eligible) return -1;
    *is_eligible = 0;
    if (residency_days < 183) return 0;
    const char *valid_rels[] = {"son", "daughter", "stepchild", "fosterchild", "brother", "sister", "nephew", "niece"};
    for (int i = 0; i < 8; i++) {
        if (strcasecmp(relationship, valid_rels[i]) == 0) {
            *is_eligible = 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_mf_imf_verify_homebuyer_recapture(int acquisition_year, double credit_received, double *recapture_amount) {
    if (!recapture_amount) return -1;
    *recapture_amount = 0.0;
    if (acquisition_year == 2008) {
        *recapture_amount = credit_received / 15.0;
    }
    return 0;
}

int tsfi_mf_cade_query_prior_refund_offset(const char *ssn, const char *offset_registry, double *offset_amount) {
    if (!ssn || !offset_amount) return -1;
    *offset_amount = 0.0;
    if (!offset_registry) return 0;
    const char *p = strstr(offset_registry, ssn);
    if (p) {
        p = strchr(p, '=');
        if (p) {
            p++;
            *offset_amount = atof(p);
        }
    }
    return 0;
}

int tsfi_mf_cade_verify_ip_pin(const char *ip_pin, int *is_valid) {
    if (!is_valid) return -1;
    if (!ip_pin || strlen(ip_pin) != 6) {
        *is_valid = 0;
        return 0;
    }
    for (int i = 0; i < 6; i++) {
        if (ip_pin[i] < '0' || ip_pin[i] > '9') {
            *is_valid = 0;
            return 0;
        }
    }
    *is_valid = 1;
    return 0;
}

int tsfi_mf_imf_verify_single_standard_deduction(double claimed_deduction, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (claimed_deduction == 13850.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_state_code(const char *state_code, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    if (!state_code || strlen(state_code) != 2) return 0;
    const char *list[] = {
        "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
        "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
        "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
        "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
        "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY"
    };
    for (int i = 0; i < 50; i++) {
        if (strcmp(state_code, list[i]) == 0) {
            *is_valid = 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_mf_imf_verify_dependent_care_ceiling(int dependent_count, double claimed_expenses, int *is_valid) {
    if (!is_valid) return -1;
    if (dependent_count <= 0) {
        *is_valid = (claimed_expenses <= 0.0) ? 1 : 0;
        return 0;
    }
    double ceiling = (dependent_count == 1) ? 3000.00 : 6000.00;
    *is_valid = (claimed_expenses <= ceiling) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_street_parity(const char *address, int *parity_out) {
    if (!parity_out) return -1;
    *parity_out = 0;
    if (!address) return 0;
    for (int i = 0; address[i] != '\0'; i++) {
        if (address[i] >= '0' && address[i] <= '9') {
            int val = 0;
            while (address[i] >= '0' && address[i] <= '9') {
                val = val * 10 + (address[i] - '0');
                i++;
            }
            *parity_out = (val % 2 == 0) ? 2 : 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_mf_imf_verify_student_loan_interest(double claimed_interest, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (claimed_interest <= 2500.00 && claimed_interest >= 0.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_deduction_bonus(int over_65_count, int blind_count, int filing_status, double claimed_bonus, int *is_valid) {
    if (!is_valid) return -1;
    double bonus_amt = (filing_status == 1 || filing_status == 4) ? 1950.00 : 1550.00;
    double calculated = (over_65_count + blind_count) * bonus_amt;
    *is_valid = (claimed_bonus == calculated) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_ss_benefits(double modified_agi, double half_ss_benefits, double taxable_ss_reported, int filing_status, int *is_valid) {
    if (!is_valid) return -1;
    double threshold = (filing_status == 2) ? 32000.00 : 25000.00;
    if (modified_agi + half_ss_benefits <= threshold) {
        *is_valid = (taxable_ss_reported == 0.0) ? 1 : 0;
    } else {
        *is_valid = (taxable_ss_reported >= 0.0) ? 1 : 0;
    }
    return 0;
}

int tsfi_mf_cade_verify_signature_date(int sign_year, int tax_year, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (sign_year >= tax_year && sign_year <= tax_year + 3) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_qbi_threshold(double taxable_income, int filing_status, int *is_below_threshold) {
    if (!is_below_threshold) return -1;
    double threshold = (filing_status == 2) ? 373200.00 : 186600.00;
    *is_below_threshold = (taxable_income <= threshold) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_state_withholding(const char *state_code, double state_wages, double claimed_withholding, int *is_valid) {
    if (!is_valid) return -1;
    (void)state_code;
    *is_valid = (claimed_withholding >= 0.0 && claimed_withholding <= state_wages && claimed_withholding <= state_wages * 0.15) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_k1_share(double partner_share_pct, double partnership_income, double reported_share_amt, int *is_valid) {
    if (!is_valid) return -1;
    double calculated = partnership_income * (partner_share_pct / 100.0);
    double diff = reported_share_amt - calculated;
    if (diff < 0) diff = -diff;
    *is_valid = (diff <= 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_witness_indicator(int doc_code, int has_witness_sig, int *is_valid) {
    if (!is_valid) return -1;
    if (doc_code == 8283) {
        *is_valid = has_witness_sig ? 1 : 0;
    } else {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_imf_verify_actc_limit(int child_count, double claimed_actc, int *is_valid) {
    if (!is_valid) return -1;
    double limit = child_count * 1700.00;
    *is_valid = (claimed_actc <= limit && claimed_actc >= 0.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_cade_verify_filing_volume(const char *state_code, int current_count, int max_limit, int *is_allowed) {
    if (!is_allowed) return -1;
    (void)state_code;
    *is_allowed = (current_count < max_limit) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_first_time_address(const char *address, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    if (!address) return 0;
    int has_digit = 0;
    int has_letter = 0;
    for (int i = 0; address[i] != '\0'; i++) {
        if (address[i] >= '0' && address[i] <= '9') has_digit = 1;
        if ((address[i] >= 'A' && address[i] <= 'Z') || (address[i] >= 'a' && address[i] <= 'z')) has_letter = 1;
    }
    *is_valid = (has_digit && has_letter) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_federal_withholding_cap(double gross_income, double claimed_withholding, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (claimed_withholding >= 0.0 && claimed_withholding <= gross_income && claimed_withholding <= gross_income * 0.50) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_check_amt_threshold(double agi, int filing_status, int *requires_amt) {
    if (!requires_amt) return -1;
    double threshold = (filing_status == 2) ? 133300.00 : 85700.00;
    *requires_amt = (agi > threshold) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_social_security_tax_cap(double wages, double claimed_ss_withheld, int *is_valid) {
    if (!is_valid) return -1;
    double limit = 160200.00 * 0.062;
    double expected = wages * 0.062;
    if (expected > limit) expected = limit;
    double diff = claimed_ss_withheld - expected;
    if (diff < 0) diff = -diff;
    *is_valid = (diff <= 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_qualified_dividend_rate(double taxable_income, double qualified_dividends, double dividend_tax, int *is_valid) {
    if (!is_valid) return -1;
    double rate = 0.15;
    if (taxable_income <= 44625.00) {
        rate = 0.0;
    } else if (taxable_income > 492300.00) {
        rate = 0.20;
    }
    double expected = qualified_dividends * rate;
    double diff = dividend_tax - expected;
    if (diff < 0) diff = -diff;
    *is_valid = (diff <= 5.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_check_se_tax_threshold(double net_profit, int *se_required) {
    if (!se_required) return -1;
    *se_required = (net_profit >= 400.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_check_niit_threshold(double magi, int filing_status, int *niit_required) {
    if (!niit_required) return -1;
    double threshold = (filing_status == 2) ? 250000.00 : 200000.00;
    *niit_required = (magi > threshold) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_medical_floor(double agi, double total_expenses, double claimed_deduction, int *is_valid) {
    if (!is_valid) return -1;
    double floor = agi * 0.075;
    double allowed = total_expenses - floor;
    if (allowed < 0.0) allowed = 0.0;
    double diff = claimed_deduction - allowed;
    if (diff < 0) diff = -diff;
    *is_valid = (diff <= 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_qbi_deduction(double qbi, double taxable_income, double claimed_qbi_deduction, int *is_valid) {
    if (!is_valid) return -1;
    double limit = (taxable_income < qbi) ? taxable_income : qbi;
    double expected = limit * 0.20;
    double diff = claimed_qbi_deduction - expected;
    if (diff < 0) diff = -diff;
    *is_valid = (diff <= 1.00) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_select_deduction(double standard_ded, double itemized_ded, double claimed_ded, int *is_valid) {
    if (!is_valid) return -1;
    double expected = (standard_ded > itemized_ded) ? standard_ded : itemized_ded;
    *is_valid = (claimed_ded == expected) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_charity_cap(double agi, double claimed_charity, int *is_valid) {
    if (!is_valid) return -1;
    double cap = agi * 0.60;
    *is_valid = (claimed_charity <= cap && claimed_charity >= 0.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_ptc_eligibility(double agi, double fpl_limit, int *is_eligible) {
    if (!is_eligible) return -1;
    *is_eligible = (agi <= fpl_limit * 4.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_foreign_tax_credit(double foreign_taxes_paid, double total_tax_liability, double claimed_credit, int *is_valid) {
    if (!is_valid) return -1;
    double allowed = (foreign_taxes_paid < total_tax_liability) ? foreign_taxes_paid : total_tax_liability;
    *is_valid = (claimed_credit <= allowed && claimed_credit >= 0.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_check_interest_limit_for_eitc(double interest_income, int *is_eligible) {
    if (!is_eligible) return -1;
    *is_eligible = (interest_income <= 11000.00 && interest_income >= 0.0) ? 1 : 0;
    return 0;
}

int tsfi_mf_imf_verify_eitc_income_limit(double earned_income, int child_count, int filing_status, int *is_eligible) {
    if (!is_eligible) return -1;
    (void)child_count;
    double limit = (filing_status == 2) ? 63398.00 : 56838.00;
    *is_eligible = (earned_income <= limit && earned_income >= 0.0) ? 1 : 0;
    return 0;
}


