#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int tsfi_cw_payroll_process_card(const char *card_line, tsfi_cw_payroll_record *rec_out) {
    if (!card_line || !rec_out) return -1;
    if (strlen(card_line) < 20) return -2;
    
    // Parse fields
    char emp_id[7];
    memcpy(emp_id, card_line, 6);
    emp_id[6] = '\0';
    strcpy(rec_out->employee_id, emp_id);
    
    char hrs_str[6];
    memcpy(hrs_str, card_line + 9, 5);
    hrs_str[5] = '\0';
    rec_out->hours_worked = atof(hrs_str);
    
    char rate_str[6];
    memcpy(rate_str, card_line + 14, 5);
    rate_str[5] = '\0';
    rec_out->hourly_rate = atof(rate_str);
    
    rec_out->exemptions = card_line[19] - '0';
    
    // Calculate Gross Pay (with time-and-a-half overtime over 40 hours)
    if (rec_out->hours_worked > 40.0) {
        double regular_pay = 40.0 * rec_out->hourly_rate;
        double overtime_pay = (rec_out->hours_worked - 40.0) * rec_out->hourly_rate * 1.5;
        rec_out->gross_pay = regular_pay + overtime_pay;
    } else {
        rec_out->gross_pay = rec_out->hours_worked * rec_out->hourly_rate;
    }
    
    // FICA withholding (3.0%)
    rec_out->fica_withholding = rec_out->gross_pay * 0.03;
    
    // Federal withholding (18% of gross minus exemptions allowance of 13.0 per exemption)
    double taxable = rec_out->gross_pay - (rec_out->exemptions * 13.0);
    if (taxable < 0.0) taxable = 0.0;
    rec_out->fed_withholding = taxable * 0.18;
    
    // Net Pay
    rec_out->net_pay = rec_out->gross_pay - rec_out->fica_withholding - rec_out->fed_withholding;
    
    return 0;
}

int tsfi_cw_ramac_process_transaction(tsfi_cw_ramac_stock *stock, const char *tx_card_line, int *reorder_triggered) {
    if (!stock || !tx_card_line || !reorder_triggered) return -1;
    if (strlen(tx_card_line) < 16) return -2;
    
    char part_id[7];
    memcpy(part_id, tx_card_line, 6);
    part_id[6] = '\0';
    if (strcmp(stock->part_id, part_id) != 0) return -3; // Mismatched part ID
    
    char action = tx_card_line[9];
    
    char qty_str[6];
    memcpy(qty_str, tx_card_line + 11, 5);
    qty_str[5] = '\0';
    int qty = atoi(qty_str);
    
    *reorder_triggered = 0;
    
    if (action == 'Q') {
        // Query only, do nothing
    } else if (action == 'S') {
        // Sale / Decrement
        if (stock->quantity_on_hand < qty) return -4; // Out of stock / underflow
        stock->quantity_on_hand -= qty;
        if (stock->quantity_on_hand < stock->reorder_point) {
            *reorder_triggered = 1;
        }
    } else if (action == 'R') {
        // Receipt / Increment
        stock->quantity_on_hand += qty;
    } else {
        return -5; // Invalid action code
    }
    
    return 0;
}

int tsfi_cw_salary_process_cards(const char **card_lines, int card_count, tsfi_cw_survey_stats *stats_out) {
    if (!card_lines || !stats_out || card_count < 0) return -1;
    
    double pg_sum = 0, op_sum = 0, sa_sum = 0;
    int pg_cnt = 0, op_cnt = 0, sa_cnt = 0;
    int f_cnt = 0, m_cnt = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *line = card_lines[i];
        if (strlen(line) < 23) continue;
        
        char role[3];
        memcpy(role, line + 9, 2);
        role[2] = '\0';
        
        char sal_str[7];
        memcpy(sal_str, line + 15, 6);
        sal_str[6] = '\0';
        double sal = atof(sal_str);
        
        char gender = line[22];
        
        if (strcmp(role, "PG") == 0) {
            pg_sum += sal;
            pg_cnt++;
        } else if (strcmp(role, "OP") == 0) {
            op_sum += sal;
            op_cnt++;
        } else if (strcmp(role, "SA") == 0) {
            sa_sum += sal;
            sa_cnt++;
        }
        
        if (gender == 'F') f_cnt++;
        else if (gender == 'M') m_cnt++;
    }
    
    stats_out->avg_programmer_salary = (pg_cnt > 0) ? (pg_sum / pg_cnt) : 0.0;
    stats_out->avg_operator_salary = (op_cnt > 0) ? (op_sum / op_cnt) : 0.0;
    stats_out->avg_analyst_salary = (sa_cnt > 0) ? (sa_sum / sa_cnt) : 0.0;
    stats_out->female_count = f_cnt;
    stats_out->male_count = m_cnt;
    
    return 0;
}

int tsfi_cw_ar_process_ledger(const char **tx_cards, int card_count, const char *ref_date_yymmdd, tsfi_cw_ar_statement *statements_out, int *stmt_count_out) {
    if (!tx_cards || !ref_date_yymmdd || !statements_out || !stmt_count_out || card_count < 0) return -1;
    if (strlen(ref_date_yymmdd) < 6) return -3;
    
    char ry_str[3], rm_str[3], rd_str[3];
    memcpy(ry_str, ref_date_yymmdd, 2); ry_str[2] = '\0';
    memcpy(rm_str, ref_date_yymmdd + 2, 2); rm_str[2] = '\0';
    memcpy(rd_str, ref_date_yymmdd + 4, 2); rd_str[2] = '\0';
    int ref_yy = atoi(ry_str);
    int ref_mm = atoi(rm_str);
    int ref_dd = atoi(rd_str);
    
    int st_count = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = tx_cards[i];
        if (strlen(card) < 22) continue;
        
        char cust_id[7];
        memcpy(cust_id, card, 6);
        cust_id[6] = '\0';
        
        // Find existing statement
        int st_idx = -1;
        for (int s = 0; s < st_count; s++) {
            if (strcmp(statements_out[s].customer_id, cust_id) == 0) {
                st_idx = s;
                break;
            }
        }
        if (st_idx == -1) {
            if (st_count >= 16) return -2; // Statement buffer limit
            st_idx = st_count++;
            strcpy(statements_out[st_idx].customer_id, cust_id);
            statements_out[st_idx].balance_current = 0;
            statements_out[st_idx].balance_30_days = 0;
            statements_out[st_idx].balance_60_days = 0;
            statements_out[st_idx].balance_90_days = 0;
            statements_out[st_idx].total_balance = 0;
        }
        
        char yy_str[3], mm_str[3], dd_str[3];
        memcpy(yy_str, card + 7, 2); yy_str[2] = '\0';
        memcpy(mm_str, card + 9, 2); mm_str[2] = '\0';
        memcpy(dd_str, card + 11, 2); dd_str[2] = '\0';
        int yy = atoi(yy_str);
        int mm = atoi(mm_str);
        int dd = atoi(dd_str);
        
        char type = card[14];
        
        char amt_str[7];
        memcpy(amt_str, card + 16, 6);
        amt_str[6] = '\0';
        double amt = atof(amt_str);
        
        // Simple approximate date diff in days
        int date_diff = (ref_yy - yy) * 360 + (ref_mm - mm) * 30 + (ref_dd - dd);
        if (date_diff < 0) date_diff = 0;
        
        if (type == 'C') {
            statements_out[st_idx].total_balance += amt;
            if (date_diff <= 30) {
                statements_out[st_idx].balance_current += amt;
            } else if (date_diff <= 60) {
                statements_out[st_idx].balance_30_days += amt;
            } else if (date_diff <= 90) {
                statements_out[st_idx].balance_60_days += amt;
            } else {
                statements_out[st_idx].balance_90_days += amt;
            }
        } else if (type == 'P') {
            statements_out[st_idx].total_balance -= amt;
            // Apply payment to reduce oldest buckets first
            double pay_rem = amt;
            if (pay_rem > 0) {
                double red = (pay_rem < statements_out[st_idx].balance_90_days) ? pay_rem : statements_out[st_idx].balance_90_days;
                statements_out[st_idx].balance_90_days -= red;
                pay_rem -= red;
            }
            if (pay_rem > 0) {
                double red = (pay_rem < statements_out[st_idx].balance_60_days) ? pay_rem : statements_out[st_idx].balance_60_days;
                statements_out[st_idx].balance_60_days -= red;
                pay_rem -= red;
            }
            if (pay_rem > 0) {
                double red = (pay_rem < statements_out[st_idx].balance_30_days) ? pay_rem : statements_out[st_idx].balance_30_days;
                statements_out[st_idx].balance_30_days -= red;
                pay_rem -= red;
            }
            if (pay_rem > 0) {
                statements_out[st_idx].balance_current -= pay_rem;
            }
        }
    }
    
    *stmt_count_out = st_count;
    return 0;
}

int tsfi_cw_gl_compile_balance_sheet(const char **tx_cards, int card_count, tsfi_cw_gl_balance_sheet *sheet_out) {
    if (!tx_cards || !sheet_out || card_count < 0) return -1;
    
    double debits_total = 0, credits_total = 0;
    double assets = 0, liabilities = 0, equity = 0;
    double revenue = 0, expense = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = tx_cards[i];
        if (strlen(card) < 15) continue;
        
        char type = card[5];
        
        char amt_str[7];
        memcpy(amt_str, card + 7, 6);
        amt_str[6] = '\0';
        double amt = atof(amt_str);
        
        char dc = card[14];
        
        if (dc == 'D') {
            debits_total += amt;
            if (type == 'A') assets += amt;
            else if (type == 'L') liabilities -= amt;
            else if (type == 'E') equity -= amt;
            else if (type == 'R') revenue -= amt;
            else if (type == 'X') expense += amt;
        } else if (dc == 'C') {
            credits_total += amt;
            if (type == 'A') assets -= amt;
            else if (type == 'L') liabilities += amt;
            else if (type == 'E') equity += amt;
            else if (type == 'R') revenue += amt;
            else if (type == 'X') expense -= amt;
        }
    }
    
    // Verify double-entry balance
    if (fabs(debits_total - credits_total) > 1e-9) {
        return -5; // Ledger out of balance error
    }
    
    sheet_out->net_income = revenue - expense;
    sheet_out->total_assets = assets;
    sheet_out->total_liabilities = liabilities;
    sheet_out->total_equity = equity + sheet_out->net_income;
    
    return 0;
}

int tsfi_cw_depreciation_calculate(const tsfi_cw_depreciation_asset *asset, char method, int target_year, double *expense_out, double *book_value_out) {
    if (!asset || !expense_out || !book_value_out || target_year <= 0) return -1;
    if (target_year > asset->useful_life) {
        *expense_out = 0.0;
        *book_value_out = asset->salvage_value;
        return 0;
    }
    
    if (method == 'S') {
        double dep_per_year = (asset->cost - asset->salvage_value) / asset->useful_life;
        *expense_out = dep_per_year;
        *book_value_out = asset->cost - (dep_per_year * target_year);
    } else if (method == 'D') {
        double rate = 2.0 / asset->useful_life;
        double current_bv = asset->cost;
        double exp = 0.0;
        
        for (int y = 1; y <= target_year; y++) {
            exp = current_bv * rate;
            if (current_bv - exp < asset->salvage_value) {
                exp = current_bv - asset->salvage_value;
            }
            current_bv -= exp;
        }
        *expense_out = exp;
        *book_value_out = current_bv;
    } else {
        return -2; // Invalid method code
    }
    
    return 0;
}

int tsfi_cw_three_way_match(const tsfi_cw_po_record *po, const tsfi_cw_receiving_record *rr, const tsfi_cw_invoice_record *invoice, tsfi_cw_match_result *result_out) {
    if (!po || !rr || !invoice || !result_out) return -1;
    
    result_out->matches = 0;
    result_out->qty_mismatch = 0;
    result_out->price_mismatch = 0;
    result_out->status_approved = 0;
    
    if (strcmp(po->po_id, rr->po_id) == 0 && strcmp(po->po_id, invoice->po_id) == 0 &&
        strcmp(po->part_id, rr->part_id) == 0 && strcmp(po->part_id, invoice->part_id) == 0) {
        result_out->matches = 1;
    } else {
        return 0; // Not matched
    }
    
    if (rr->quantity_received != po->quantity || invoice->quantity_invoiced != po->quantity) {
        result_out->qty_mismatch = 1;
    }
    
    double expected_amt = invoice->quantity_invoiced * po->unit_price;
    if (fabs(invoice->invoice_amount - expected_amt) > 1e-2) {
        result_out->price_mismatch = 1;
    }
    
    if (result_out->qty_mismatch == 0 && result_out->price_mismatch == 0) {
        result_out->status_approved = 1;
    }
    
    return 0;
}
