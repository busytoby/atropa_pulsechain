#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Include modular mainframe emulator subcomponents
#include "tsfi_mainframe_y2k.c"
#include "tsfi_mainframe_ebcdic.c"
#include "tsfi_mainframe_cobol.c"
#include "tsfi_mainframe_vsam.c"
#include "tsfi_mainframe_jcl.c"

#include <math.h>

int tsfi_b5500_prt_resolve_descriptor(uint64_t descriptor, uint32_t offset, const uint64_t *memory_pool, uint32_t mem_size, uint64_t *val_out) {
    if (!memory_pool || !val_out) return -1;
    int present = (descriptor & (1ULL << 47)) ? 1 : 0;
    if (!present) return -5;
    uint32_t limit = (descriptor >> 30) & 0xFFFF;
    if (offset >= limit) return -6;
    uint32_t origin = descriptor & 0x3FFFFFFF;
    if (origin + offset >= mem_size) return -7;
    *val_out = memory_pool[origin + offset];
    return 0;
}

int tsfi_cw_honeywell_bcd_to_ebcdic(const uint8_t *bcd_in, int bcd_len, uint8_t *ebcdic_out) {
    if (!bcd_in || !ebcdic_out || bcd_len < 0) return -1;
    static const uint8_t bcd_to_ebcdic_map[64] = {
        0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
        0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
        0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
        0xE7, 0xE8, 0xE9, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
        0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x4B, 0x4C, 0x4D,
        0x4E, 0x4F, 0x50, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
        0x5F, 0x60, 0x61, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x50, 0x7D
    };
    for (int i = 0; i < bcd_len; i++) {
        uint8_t code = bcd_in[i] & 0x3F;
        ebcdic_out[i] = bcd_to_ebcdic_map[code];
    }
    return 0;
}

double tsfi_cw_univac_to_double(uint64_t univac_val) {
    int sign = (univac_val & (1ULL << 35)) ? 1 : 0;
    int exponent = (univac_val >> 27) & 0xFF;
    uint64_t mantissa = univac_val & 0x7FFFFFFULL;
    if (exponent == 0 && mantissa == 0) return 0.0;
    int true_exp = exponent - 128;
    double frac = (double)mantissa / (double)(1ULL << 27);
    double result = frac * pow(2.0, true_exp);
    return sign ? -result : result;
}

uint64_t tsfi_cw_double_to_univac(double val) {
    if (val == 0.0) return 0ULL;
    uint64_t sign = 0;
    if (val < 0.0) {
        sign = 1ULL << 35;
        val = -val;
    }
    int exponent = 0;
    double frac = frexp(val, &exponent);
    int biased_exp = exponent + 128;
    if (biased_exp < 0) biased_exp = 0;
    if (biased_exp > 255) biased_exp = 255;
    uint64_t mantissa = (uint64_t)(frac * (double)(1ULL << 27));
    if (mantissa > 0x7FFFFFFULL) mantissa = 0x7FFFFFFULL;
    return sign | ((uint64_t)biased_exp << 27) | mantissa;
}

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

int tsfi_cw_simplex_optimize(const tsfi_cw_simplex_problem *prob, double *x1_opt, double *x2_opt, double *profit_opt) {
    if (!prob || !x1_opt || !x2_opt || !profit_opt) return -1;
    
    double max_profit = -1.0;
    double best_x1 = 0.0, best_x2 = 0.0;
    
    // Corner Points to evaluate:
    double corners[6][2] = {
        {0.0, 0.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0}
    };
    
    // Corner 1: (b1 / a11, 0)
    if (prob->a11 > 0.0) {
        corners[1][0] = prob->b1 / prob->a11;
        corners[1][1] = 0.0;
    }
    // Corner 2: (0, b1 / a12)
    if (prob->a12 > 0.0) {
        corners[2][0] = 0.0;
        corners[2][1] = prob->b1 / prob->a12;
    }
    // Corner 3: (b2 / a21, 0)
    if (prob->a21 > 0.0) {
        corners[3][0] = prob->b2 / prob->a21;
        corners[3][1] = 0.0;
    }
    // Corner 4: (0, b2 / a22)
    if (prob->a22 > 0.0) {
        corners[4][0] = 0.0;
        corners[4][1] = prob->b2 / prob->a22;
    }
    // Corner 5: Intersection of Constraint 1 and Constraint 2
    double det = (prob->a11 * prob->a22) - (prob->a12 * prob->a21);
    if (fabs(det) > 1e-9) {
        corners[5][0] = ((prob->b1 * prob->a22) - (prob->a12 * prob->b2)) / det;
        corners[5][1] = ((prob->a11 * prob->b2) - (prob->b1 * prob->a21)) / det;
    }
    
    for (int i = 0; i < 6; i++) {
        double x1 = corners[i][0];
        double x2 = corners[i][1];
        
        if (x1 < 0.0 || x2 < 0.0) continue;
        
        // Check constraints (with a small margin for double precision issues)
        if ((prob->a11 * x1 + prob->a12 * x2) > prob->b1 + 1e-9) continue;
        if ((prob->a21 * x1 + prob->a22 * x2) > prob->b2 + 1e-9) continue;
        
        double profit = prob->c1 * x1 + prob->c2 * x2;
        if (profit > max_profit) {
            max_profit = profit;
            best_x1 = x1;
            best_x2 = x2;
        }
    }
    
    *x1_opt = best_x1;
    *x2_opt = best_x2;
    *profit_opt = max_profit;
    
    return 0;
}

int tsfi_cw_cpm_schedule(tsfi_cw_cpm_task *tasks, int task_count) {
    if (!tasks || task_count <= 0) return -1;
    
    // 1. Forward Pass
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].pred_count == 0) {
            tasks[i].early_start = 0;
        } else {
            int max_ef = 0;
            for (int p = 0; p < tasks[i].pred_count; p++) {
                int pred_id = tasks[i].predecessors[p];
                for (int j = 0; j < i; j++) {
                    if (tasks[j].id == pred_id && tasks[j].early_finish > max_ef) {
                        max_ef = tasks[j].early_finish;
                    }
                }
            }
            tasks[i].early_start = max_ef;
        }
        tasks[i].early_finish = tasks[i].early_start + tasks[i].duration;
    }
    
    // Find project completion time
    int project_duration = 0;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].early_finish > project_duration) {
            project_duration = tasks[i].early_finish;
        }
    }
    
    // 2. Backward Pass
    for (int i = task_count - 1; i >= 0; i--) {
        // Find if this task is a predecessor to any task
        int is_pred = 0;
        int min_ls = 999999;
        for (int j = i + 1; j < task_count; j++) {
            for (int p = 0; p < tasks[j].pred_count; p++) {
                if (tasks[j].predecessors[p] == tasks[i].id) {
                    is_pred = 1;
                    if (tasks[j].late_start < min_ls) {
                        min_ls = tasks[j].late_start;
                    }
                }
            }
        }
        if (!is_pred) {
            tasks[i].late_finish = project_duration;
        } else {
            tasks[i].late_finish = min_ls;
        }
        tasks[i].late_start = tasks[i].late_finish - tasks[i].duration;
        tasks[i].slack = tasks[i].late_start - tasks[i].early_start;
        tasks[i].is_critical = (tasks[i].slack == 0) ? 1 : 0;
    }
    
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

int tsfi_cw_eoq_calculate(const tsfi_cw_eoq_problem *prob, double *eoq_out, double *total_cost_out) {
    if (!prob || !eoq_out || !total_cost_out) return -1;
    if (prob->annual_demand <= 0.0 || prob->order_cost <= 0.0 || prob->holding_cost <= 0.0) return -2;
    
    *eoq_out = sqrt((2.0 * prob->annual_demand * prob->order_cost) / prob->holding_cost);
    
    double order_tc = (prob->annual_demand / (*eoq_out)) * prob->order_cost;
    double holding_tc = ((*eoq_out) / 2.0) * prob->holding_cost;
    *total_cost_out = order_tc + holding_tc;
    
    return 0;
}

int tsfi_cw_pert_calculate(tsfi_cw_pert_task *tasks, int task_count, double *expected_project_length_out, double *project_variance_out) {
    if (!tasks || task_count <= 0 || !expected_project_length_out || !project_variance_out) return -1;
    
    // Calculate expected duration and variance for each task
    for (int i = 0; i < task_count; i++) {
        tasks[i].expected_duration = (tasks[i].opt_duration + 4.0 * tasks[i].most_likely + tasks[i].pess_duration) / 6.0;
        tasks[i].variance = pow((tasks[i].pess_duration - tasks[i].opt_duration) / 6.0, 2.0);
    }
    
    // Forward Pass
    double *es = (double *)calloc(task_count, sizeof(double));
    double *ef = (double *)calloc(task_count, sizeof(double));
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].pred_count == 0) {
            es[i] = 0.0;
        } else {
            double max_ef = 0.0;
            for (int p = 0; p < tasks[i].pred_count; p++) {
                int pred_id = tasks[i].predecessors[p];
                for (int j = 0; j < i; j++) {
                    if (tasks[j].id == pred_id && ef[j] > max_ef) {
                        max_ef = ef[j];
                    }
                }
            }
            es[i] = max_ef;
        }
        ef[i] = es[i] + tasks[i].expected_duration;
    }
    
    double project_length = 0.0;
    for (int i = 0; i < task_count; i++) {
        if (ef[i] > project_length) {
            project_length = ef[i];
        }
    }
    
    // Backward Pass
    double *ls = (double *)calloc(task_count, sizeof(double));
    double *lf = (double *)calloc(task_count, sizeof(double));
    for (int i = task_count - 1; i >= 0; i--) {
        int is_pred = 0;
        double min_ls = 999999.0;
        for (int j = i + 1; j < task_count; j++) {
            for (int p = 0; p < tasks[j].pred_count; p++) {
                if (tasks[j].predecessors[p] == tasks[i].id) {
                    is_pred = 1;
                    if (ls[j] < min_ls) {
                        min_ls = ls[j];
                    }
                }
            }
        }
        if (!is_pred) {
            lf[i] = project_length;
        } else {
            lf[i] = min_ls;
        }
        ls[i] = lf[i] - tasks[i].expected_duration;
    }
    
    // Sum variance of critical path tasks (slack is zero)
    double total_variance = 0.0;
    for (int i = 0; i < task_count; i++) {
        double slack = lf[i] - ef[i];
        if (fabs(slack) < 1e-5) {
            total_variance += tasks[i].variance;
        }
    }
    
    *expected_project_length_out = project_length;
    *project_variance_out = total_variance;
    
    free(es);
    free(ef);
    free(ls);
    free(lf);
    
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

int tsfi_cw_transport_optimize(const tsfi_cw_transportation_problem *prob, double shipment[2][2], double *total_cost_out) {
    if (!prob || !shipment || !total_cost_out) return -1;
    
    // Check feasibility: total supply must equal total demand for this simplified solver
    double total_supply = prob->supply[0] + prob->supply[1];
    double total_demand = prob->demand[0] + prob->demand[1];
    if (fabs(total_supply - total_demand) > 1e-9) return -2;
    
    double s1 = prob->supply[0];
    double s2 = prob->supply[1];
    double d1 = prob->demand[0];
    
    // Bounds on x11: max(0, d1 - s2) <= x11 <= min(d1, s1)
    double x11_min = (0.0 > (d1 - s2)) ? 0.0 : (d1 - s2);
    double x11_max = (d1 < s1) ? d1 : s1;
    
    if (x11_min > x11_max) return -3; // Infeasible bounds
    
    // Coefficients of x11 in cost function: C11 - C12 - C21 + C22
    double coeff = prob->cost[0][0] - prob->cost[0][1] - prob->cost[1][0] + prob->cost[1][1];
    
    double best_x11 = (coeff < 0.0) ? x11_max : x11_min;
    
    shipment[0][0] = best_x11;
    shipment[0][1] = s1 - best_x11;
    shipment[1][0] = d1 - best_x11;
    shipment[1][1] = s2 - (d1 - best_x11);
    
    *total_cost_out = 0.0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            *total_cost_out += shipment[i][j] * prob->cost[i][j];
        }
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

int tsfi_cw_resource_level(tsfi_cw_leveling_task *tasks, int task_count, int project_horizon, int *peak_resource_out) {
    if (!tasks || task_count <= 0 || project_horizon <= 0 || !peak_resource_out) return -1;
    
    // Set default starts to early starts
    for (int i = 0; i < task_count; i++) {
        tasks[i].scheduled_start = tasks[i].early_start;
    }
    
    int *best_starts = (int *)malloc(task_count * sizeof(int));
    for (int i = 0; i < task_count; i++) {
        best_starts[i] = tasks[i].early_start;
    }
    
    int min_peak = 999999;
    int *daily = (int *)calloc(project_horizon + 1, sizeof(int));
    
    // Evaluate early start peak first
    for (int d = 0; d < project_horizon; d++) daily[d] = 0;
    for (int i = 0; i < task_count; i++) {
        int start = tasks[i].early_start;
        for (int d = start; d < start + tasks[i].duration && d < project_horizon; d++) {
            daily[d] += tasks[i].resource_rate;
        }
    }
    int current_peak = 0;
    for (int d = 0; d < project_horizon; d++) {
        if (daily[d] > current_peak) current_peak = daily[d];
    }
    min_peak = current_peak;
    
    // A simple search: for each task with slack, try shifting it
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].late_start > tasks[i].early_start) {
            for (int shift = tasks[i].early_start; shift <= tasks[i].late_start; shift++) {
                // Clear daily profile
                for (int d = 0; d < project_horizon; d++) daily[d] = 0;
                
                // Add all other tasks at their current best schedule
                for (int j = 0; j < task_count; j++) {
                    int start = (j == i) ? shift : best_starts[j];
                    for (int d = start; d < start + tasks[j].duration && d < project_horizon; d++) {
                        daily[d] += tasks[j].resource_rate;
                    }
                }
                
                int peak = 0;
                for (int d = 0; d < project_horizon; d++) {
                    if (daily[d] > peak) peak = daily[d];
                }
                
                if (peak < min_peak) {
                    min_peak = peak;
                    best_starts[i] = shift;
                }
            }
        }
    }
    
    for (int i = 0; i < task_count; i++) {
        tasks[i].scheduled_start = best_starts[i];
    }
    
    *peak_resource_out = min_peak;
    
    free(best_starts);
    free(daily);
    return 0;
}

int tsfi_cw_rop_calculate(const tsfi_cw_rop_problem *prob, double *safety_stock_out, double *reorder_point_out) {
    if (!prob || !safety_stock_out || !reorder_point_out) return -1;
    if (prob->avg_daily_demand < 0.0 || prob->avg_lead_time_days < 0.0) return -2;
    
    double term1 = prob->avg_lead_time_days * pow(prob->demand_std_dev, 2.0);
    double term2 = pow(prob->avg_daily_demand, 2.0) * pow(prob->lead_time_std_dev, 2.0);
    
    *safety_stock_out = prob->service_factor_z * sqrt(term1 + term2);
    *reorder_point_out = (prob->avg_daily_demand * prob->avg_lead_time_days) + (*safety_stock_out);
    
    return 0;
}

int tsfi_cw_icp_register_product(tsfi_cw_icp_product *catalog, int *catalog_size, const tsfi_cw_icp_product *new_prod) {
    if (!catalog || !catalog_size || !new_prod) return -1;
    
    // Check if exists
    for (int i = 0; i < *catalog_size; i++) {
        if (strcmp(catalog[i].product_id, new_prod->product_id) == 0) {
            catalog[i] = *new_prod;
            return 0;
        }
    }
    
    // Append new
    if (*catalog_size >= 32) return -3;
    catalog[*catalog_size] = *new_prod;
    (*catalog_size)++;
    return 0;
}

int tsfi_cw_icp_check_award(const tsfi_cw_icp_product *prod, tsfi_cw_icp_award_status *status_out) {
    if (!prod || !status_out) return -1;
    
    status_out->total_revenue = prod->unit_price * prod->install_count;
    status_out->qualifies_for_million_dollar_award = (status_out->total_revenue >= 1000000.0) ? 1 : 0;
    
    return 0;
}

int tsfi_cw_icp_process_agent_sale(tsfi_cw_icp_agent *agent, const char *sale_card_line) {
    if (!agent || !sale_card_line) return -1;
    if (strlen(sale_card_line) < 13) return -2;
    
    char agent_id[7];
    memcpy(agent_id, sale_card_line, 6);
    agent_id[6] = '\0';
    if (strcmp(agent->agent_id, agent_id) != 0) return -3;
    
    char amt_str[7];
    memcpy(amt_str, sale_card_line + 7, 6);
    amt_str[6] = '\0';
    double sale_amt = atof(amt_str);
    
    agent->total_sales += sale_amt;
    
    // Tiered commission: 5% up to 50,000, 10% on remainder
    if (agent->total_sales <= 50000.0) {
        agent->commission_earned = agent->total_sales * 0.05;
    } else {
        agent->commission_earned = (50000.0 * 0.05) + ((agent->total_sales - 50000.0) * 0.10);
    }
    
    agent->total_payout = agent->base_salary + agent->commission_earned;
    return 0;
}

int tsfi_cw_icp_audit_contract(const tsfi_cw_icp_contract *contract, double *total_value_out, double *remaining_value_out) {
    if (!contract || !total_value_out || !remaining_value_out) return -1;
    
    *total_value_out = contract->monthly_fee * contract->contract_months;
    
    int rem_months = contract->contract_months - contract->months_elapsed;
    if (rem_months < 0) rem_months = 0;
    *remaining_value_out = contract->monthly_fee * rem_months;
    
    return 0;
}

int tsfi_cw_icp_audit_compatibility(const tsfi_cw_icp_product *prod, const char *target_hardware, int *is_compatible_out) {
    if (!prod || !target_hardware || !is_compatible_out) return -1;
    
    if (strstr(target_hardware, prod->hardware_platform) || strstr(prod->hardware_platform, target_hardware)) {
        *is_compatible_out = 1;
    } else {
        *is_compatible_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_distribute_royalties(const tsfi_cw_icp_product *prod, double rate, double *royalty_out) {
    if (!prod || !royalty_out || rate < 0.0 || rate > 1.0) return -1;
    
    double revenue = prod->unit_price * prod->install_count;
    *royalty_out = revenue * rate;
    return 0;
}

int tsfi_cw_icp_search_directory(const tsfi_cw_icp_product *catalog, int catalog_size, const char *hardware_filter, tsfi_cw_icp_product *results_out, int *results_count_out) {
    if (!catalog || catalog_size < 0 || !results_out || !results_count_out) return -1;
    
    *results_count_out = 0;
    for (int i = 0; i < catalog_size; i++) {
        if (!hardware_filter || strlen(hardware_filter) == 0 ||
            strstr(catalog[i].hardware_platform, hardware_filter) ||
            strstr(hardware_filter, catalog[i].hardware_platform)) {
            results_out[*results_count_out] = catalog[i];
            (*results_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_icp_migration_audit(const tsfi_cw_icp_product *old_prod, const tsfi_cw_icp_product *new_prod, int *upgrade_allowed_out) {
    if (!old_prod || !new_prod || !upgrade_allowed_out) return -1;
    
    if (strcmp(old_prod->vendor, new_prod->vendor) == 0 && new_prod->unit_price >= old_prod->unit_price) {
        *upgrade_allowed_out = 1;
    } else {
        *upgrade_allowed_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_audit_license_compliance(int purchased, int active, int *warning_out) {
    if (!warning_out) return -1;
    *warning_out = (active > purchased) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_track_quota(double target, double actual, double *bonus_payout_out) {
    if (!bonus_payout_out || target <= 0.0) return -1;
    
    if (actual < target) {
        *bonus_payout_out = 0.0;
    } else if (actual < target * 1.20) {
        *bonus_payout_out = actual * 0.02;
    } else {
        *bonus_payout_out = actual * 0.05;
    }
    return 0;
}

int tsfi_cw_icp_calculate_bundle_price(const tsfi_cw_icp_product *products, int count, double discount_rate, double *total_price_out) {
    if (!products || count <= 0 || !total_price_out || discount_rate < 0.0 || discount_rate > 1.0) return -1;
    
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += products[i].unit_price;
    }
    *total_price_out = sum * (1.0 - discount_rate);
    return 0;
}

int tsfi_cw_icp_subscription_status(const tsfi_cw_icp_subscription *sub, double *total_paid_out, double *remaining_obligations_out) {
    if (!sub || !total_paid_out || !remaining_obligations_out) return -1;
    
    *total_paid_out = sub->monthly_rate * sub->cycles_paid;
    
    int rem = sub->billing_cycles_total - sub->cycles_paid;
    if (rem < 0) rem = 0;
    *remaining_obligations_out = sub->monthly_rate * rem;
    return 0;
}

int tsfi_cw_icp_analyze_vendor(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor_name, double *total_revenue_out, int *award_count_out) {
    if (!catalog || catalog_size < 0 || !vendor_name || !total_revenue_out || !award_count_out) return -1;
    
    *total_revenue_out = 0.0;
    *award_count_out = 0;
    for (int i = 0; i < catalog_size; i++) {
        if (strcmp(catalog[i].vendor, vendor_name) == 0) {
            double rev = catalog[i].unit_price * catalog[i].install_count;
            *total_revenue_out += rev;
            if (rev >= 1000000.0) {
                (*award_count_out)++;
            }
        }
    }
    return 0;
}

int tsfi_cw_icp_calculate_transfer_tax(double license_amount, double tax_rate, double *tax_amount_out) {
    if (license_amount < 0.0 || tax_rate < 0.0 || !tax_amount_out) return -1;
    *tax_amount_out = license_amount * tax_rate;
    return 0;
}

int tsfi_cw_icp_calculate_software_depreciation(double cost, double salvage_value, int lifespan_years, int target_year, double *depreciated_value_out) {
    if (cost < 0.0 || salvage_value < 0.0 || lifespan_years <= 0 || target_year < 0 || !depreciated_value_out) return -1;
    
    if (target_year >= lifespan_years) {
        *depreciated_value_out = salvage_value;
    } else {
        double annual_dep = (cost - salvage_value) / lifespan_years;
        *depreciated_value_out = cost - (annual_dep * target_year);
    }
    return 0;
}

int tsfi_cw_icp_calculate_renewal_penalty(double renewal_fee, int days_late, double penalty_rate_per_day, double *total_due_out) {
    if (renewal_fee < 0.0 || penalty_rate_per_day < 0.0 || !total_due_out) return -1;
    
    if (days_late <= 0) {
        *total_due_out = renewal_fee;
    } else {
        *total_due_out = renewal_fee * (1.0 + ((double)days_late * penalty_rate_per_day));
    }
    return 0;
}

int tsfi_cw_icp_audit_transfer(tsfi_cw_icp_product *prod, const char *new_vendor, double transfer_fee_rate, double *transfer_fee_out) {
    if (!prod || !new_vendor || transfer_fee_rate < 0.0 || !transfer_fee_out) return -1;
    
    *transfer_fee_out = prod->unit_price * transfer_fee_rate;
    strncpy(prod->vendor, new_vendor, sizeof(prod->vendor) - 1);
    prod->vendor[sizeof(prod->vendor) - 1] = '\0';
    return 0;
}

int tsfi_cw_icp_consolidate_royalties(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor, double royalty_rate, double *consolidated_payment_out) {
    if (!catalog || catalog_size < 0 || !vendor || royalty_rate < 0.0 || !consolidated_payment_out) return -1;
    
    *consolidated_payment_out = 0.0;
    for (int i = 0; i < catalog_size; i++) {
        if (strcmp(catalog[i].vendor, vendor) == 0) {
            *consolidated_payment_out += (catalog[i].unit_price * catalog[i].install_count * royalty_rate);
        }
    }
    return 0;
}

int tsfi_cw_icp_check_expiration(int current_year, int current_month, int current_day, int exp_year, int exp_month, int exp_day, int *expired_out, int *days_remaining_out) {
    if (!expired_out || !days_remaining_out) return -1;
    
    long long days_cur = (long long)current_year * 365 + (long long)current_month * 30 + current_day;
    long long days_exp = (long long)exp_year * 365 + (long long)exp_month * 30 + exp_day;
    
    *days_remaining_out = (int)(days_exp - days_cur);
    *expired_out = (*days_remaining_out < 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_calculate_payback(double purchase_cost, double annual_savings, double *payback_years_out) {
    if (purchase_cost < 0.0 || annual_savings <= 0.0 || !payback_years_out) return -1;
    *payback_years_out = purchase_cost / annual_savings;
    return 0;
}

int tsfi_cw_icp_register_release(tsfi_cw_icp_release_record *releases, int *release_count, const tsfi_cw_icp_release_record *new_release) {
    if (!releases || !release_count || !new_release) return -1;
    
    // Check if exists
    for (int i = 0; i < *release_count; i++) {
        if (strcmp(releases[i].product_id, new_release->product_id) == 0 &&
            strcmp(releases[i].version_string, new_release->version_string) == 0) {
            releases[i] = *new_release;
            return 0;
        }
    }
    
    // Append new
    if (*release_count >= 32) return -3;
    releases[*release_count] = *new_release;
    (*release_count)++;
    return 0;
}

int tsfi_cw_icp_audit_vendor(const tsfi_cw_icp_vendor_record *vendor, int *is_compliant_out) {
    if (!vendor || !is_compliant_out) return -1;
    
    if (vendor->is_active && (strcmp(vendor->country_code, "US") == 0 || strcmp(vendor->country_code, "UK") == 0)) {
        *is_compliant_out = 1;
    } else {
        *is_compliant_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_assign_category(tsfi_cw_icp_category_map *maps, int *map_count, const char *prod_id, const char *category) {
    if (!maps || !map_count || !prod_id || !category) return -1;
    
    for (int i = 0; i < *map_count; i++) {
        if (strcmp(maps[i].product_id, prod_id) == 0) {
            strncpy(maps[i].category_name, category, sizeof(maps[i].category_name) - 1);
            maps[i].category_name[sizeof(maps[i].category_name) - 1] = '\0';
            return 0;
        }
    }
    
    if (*map_count >= 32) return -3;
    strncpy(maps[*map_count].product_id, prod_id, sizeof(maps[*map_count].product_id) - 1);
    maps[*map_count].product_id[sizeof(maps[*map_count].product_id) - 1] = '\0';
    strncpy(maps[*map_count].category_name, category, sizeof(maps[*map_count].category_name) - 1);
    maps[*map_count].category_name[sizeof(maps[*map_count].category_name) - 1] = '\0';
    (*map_count)++;
    return 0;
}

int tsfi_cw_icp_query_category(const tsfi_cw_icp_category_map *maps, int map_count, const char *category, char ids_out[][8], int *ids_count_out) {
    if (!maps || map_count < 0 || !category || !ids_out || !ids_count_out) return -1;
    
    *ids_count_out = 0;
    for (int i = 0; i < map_count; i++) {
        if (strcmp(maps[i].category_name, category) == 0) {
            strncpy(ids_out[*ids_count_out], maps[i].product_id, 7);
            ids_out[*ids_count_out][7] = '\0';
            (*ids_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_icp_audit_grace_period(int days_late, int allowed_grace_days, int *suspended_out) {
    if (!suspended_out) return -1;
    *suspended_out = (days_late > allowed_grace_days) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_calculate_upgrade_price(double current_version_price, double new_version_price, double loyalty_discount_rate, double *upgrade_price_out) {
    if (current_version_price < 0.0 || new_version_price < 0.0 || loyalty_discount_rate < 0.0 || loyalty_discount_rate > 1.0 || !upgrade_price_out) return -1;
    
    double diff = new_version_price - current_version_price;
    if (diff < 0.0) diff = 0.0;
    *upgrade_price_out = diff * (1.0 - loyalty_discount_rate);
    return 0;
}

int tsfi_cw_icp_audit_support_sla(int response_time_mins, int target_sla_mins, double contract_monthly_fee, double *rebate_out) {
    if (response_time_mins < 0 || target_sla_mins <= 0 || contract_monthly_fee < 0.0 || !rebate_out) return -1;
    
    if (response_time_mins > target_sla_mins) {
        *rebate_out = contract_monthly_fee * 0.05;
    } else {
        *rebate_out = 0.0;
    }
    return 0;
}

int tsfi_cw_esj_analyze_paging(const tsfi_cw_esj_paging_metrics *metrics, double *thrashing_index_out, int *alert_flag_out) {
    if (!metrics || !thrashing_index_out || !alert_flag_out) return -1;
    if (metrics->total_frames < 0 || metrics->free_frames < 0 || metrics->free_frames > metrics->total_frames) return -2;
    
    int active_frames = metrics->total_frames - metrics->free_frames;
    *thrashing_index_out = (double)(metrics->page_ins + metrics->page_outs) / (double)(active_frames + 1);
    
    if (metrics->uic < 30 || *thrashing_index_out > 0.05) {
        *alert_flag_out = 1;
    } else {
        *alert_flag_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_optimize_migration(const tsfi_cw_marist_zvm_migration *config, double *est_time_out, int *can_migrate_out) {
    if (!config || !est_time_out || !can_migrate_out) return -1;
    if (config->vm_memory_mb <= 0.0 || config->network_speed_mbps <= 0.0) return -2;
    
    double net_speed_mb = config->network_speed_mbps / 8.0;
    double net_rate = net_speed_mb - config->dirty_rate_mb_per_sec;
    
    if (net_rate <= 0.0) {
        *est_time_out = -1.0;
        *can_migrate_out = 0;
    } else {
        *est_time_out = config->vm_memory_mb / net_rate;
        *can_migrate_out = (*est_time_out <= config->max_migration_time_sec) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_marist_audit_sdn(const tsfi_cw_marist_sdn_rule *rules, int rule_count, const char *src_ip, const char *dest_ip, int *action_out) {
    if (!rules || rule_count < 0 || !src_ip || !dest_ip || !action_out) return -1;
    
    for (int i = 0; i < rule_count; i++) {
        if (strcmp(rules[i].src_ip, src_ip) == 0 && strcmp(rules[i].dest_ip, dest_ip) == 0) {
            *action_out = rules[i].action;
            return 0;
        }
    }
    *action_out = 2; // Default DROP
    return 0;
}

int tsfi_cw_marist_audit_tenant(const tsfi_cw_marist_tenant *tenant, int *is_nominal_out) {
    if (!tenant || !is_nominal_out) return -1;
    
    if (tenant->used_cores <= tenant->allocated_cores && tenant->used_mem_gb <= tenant->allocated_mem_gb) {
        *is_nominal_out = 1;
    } else {
        *is_nominal_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_audit_isolation(const tsfi_cw_marist_guest_profile *profile, int *is_secure_out) {
    if (!profile || !is_secure_out) return -1;
    
    *is_secure_out = (profile->has_read_any_spool == 0 && (profile->privilege_class == 'G' || profile->allow_inter_vm_comm == 0)) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_sysplex(const tsfi_cw_marist_sysplex_cf *cf, int *alert_out) {
    if (!cf || !alert_out) return -1;
    
    if (cf->failed_requests > 10 || cf->avg_response_time_us > 500 || cf->buffer_util_percent > 90.0) {
        *alert_out = 1;
    } else {
        *alert_out = 0;
    }
    return 0;
}

int tsfi_cw_marist_calc_cpu_shares(const tsfi_cw_marist_zvm_scheduler *sched, double share_allocs_out[16]) {
    if (!sched || !share_allocs_out) return -1;
    if (sched->vm_count < 0 || sched->vm_count > 16) return -2;
    
    for (int i = 0; i < sched->vm_count; i++) {
        if (sched->total_shares > 0) {
            share_allocs_out[i] = (double)sched->shares[i] / (double)sched->total_shares;
        } else {
            share_allocs_out[i] = 0.0;
        }
    }
    return 0;
}

int tsfi_cw_marist_audit_rmf_cpu(const tsfi_cw_marist_rmf_cpu *cpus, int cpu_count, double *avg_busy_out, int *overloaded_out) {
    if (!cpus || cpu_count <= 0 || !avg_busy_out || !overloaded_out) return -1;
    
    double total_busy = 0.0;
    *overloaded_out = 0;
    
    for (int i = 0; i < cpu_count; i++) {
        total_busy += cpus[i].busy_percent;
        if (cpus[i].dispatch_delay_ms > 20 && cpus[i].busy_percent > 95.0) {
            *overloaded_out = 1;
        }
    }
    *avg_busy_out = total_busy / cpu_count;
    return 0;
}

int tsfi_cw_marist_optimize_vswitch(const tsfi_cw_marist_vswitch *vsw, int *needs_failover_out) {
    if (!vsw || !needs_failover_out) return -1;
    
    *needs_failover_out = (vsw->drop_rate_percent > 2.0 && vsw->backup_ports_configured > 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_wlm(const tsfi_cw_marist_wlm_service *srv, int *needs_adjustment_out) {
    if (!srv || !needs_adjustment_out) return -1;
    
    double velocity = 100.0 * srv->active_states_count / (srv->active_states_count + srv->delay_states_count + 1);
    *needs_adjustment_out = (velocity < srv->goal_velocity) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_alloc_crypto(const tsfi_cw_marist_crypto_coproc *cfg, double guest_allocs_out[16]) {
    if (!cfg || !guest_allocs_out) return -1;
    if (cfg->guest_count < 0 || cfg->guest_count > 16) return -2;
    
    for (int i = 0; i < cfg->guest_count; i++) {
        if (cfg->total_crypto_units > 0) {
            guest_allocs_out[i] = (double)cfg->guest_shares[i] / (double)cfg->total_crypto_units;
        } else {
            guest_allocs_out[i] = 0.0;
        }
    }
    return 0;
}

int tsfi_cw_marist_audit_minidisk_cache(const tsfi_cw_marist_minidisk_cache *cache, double *hit_ratio_out, int *needs_resize_out) {
    if (!cache || !hit_ratio_out || !needs_resize_out) return -1;
    
    *hit_ratio_out = (cache->read_requests > 0) ? (double)cache->read_hits / (double)cache->read_requests : 0.0;
    *needs_resize_out = (*hit_ratio_out < 0.60 && cache->read_requests > 100) ? 1 : 0;
    return 0;
}

int tsfi_cw_marist_audit_isglock(const tsfi_cw_marist_isglock *lock, int *alert_out) {
    if (!lock || !alert_out) return -1;
    
    double contention_ratio = (lock->lock_requests > 0) ? (double)lock->contended_requests / lock->lock_requests : 0.0;
    if (lock->timeouts > 0 || lock->avg_grant_delay_ms > 50 || contention_ratio > 0.15) {
        *alert_out = 1;
    } else {
        *alert_out = 0;
    }
    return 0;
}

int tsfi_cw_niu_hlasm_exec(uint32_t regs[16], const char *op, int r1, int r2, int32_t immediate, int *overflow_out) {
    if (!regs || !op || !overflow_out) return -1;
    if (r1 < 0 || r1 >= 16 || r2 < 0 || r2 >= 16) return -2;
    
    *overflow_out = 0;
    if (strcmp(op, "AR") == 0) {
        uint64_t res = (uint64_t)regs[r1] + regs[r2];
        regs[r1] = (uint32_t)res;
        *overflow_out = (res > 0xFFFFFFFFULL) ? 1 : 0;
    } else if (strcmp(op, "SR") == 0) {
        regs[r1] = regs[r1] - regs[r2];
    } else if (strcmp(op, "L") == 0) {
        regs[r1] = (uint32_t)immediate;
    } else if (strcmp(op, "ST") == 0) {
        // Simulated store
    } else {
        return -3; // Unknown opcode
    }
    return 0;
}

int tsfi_cw_niu_validate_jcl(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = (strncmp(jcl_line, "//", 2) == 0 && (strstr(jcl_line, " JOB ") != NULL || strstr(jcl_line, " EXEC ") != NULL || strstr(jcl_line, " DD ") != NULL)) ? 1 : 0;
    return 0;
}

int tsfi_cw_niu_expand_macro(const char *macro_def, const char *param, char *output_out, size_t max_len) {
    if (!macro_def || !param || !output_out || max_len == 0) return -1;
    
    const char *placeholder = "&PARAM";
    const char *found = strstr(macro_def, placeholder);
    if (!found) {
        strncpy(output_out, macro_def, max_len - 1);
        output_out[max_len - 1] = '\0';
        return 0;
    }
    
    size_t prefix_len = found - macro_def;
    if (prefix_len >= max_len) prefix_len = max_len - 1;
    
    memcpy(output_out, macro_def, prefix_len);
    output_out[prefix_len] = '\0';
    
    if (strlen(output_out) + strlen(param) < max_len) {
        strcat(output_out, param);
    }
    
    const char *suffix = found + strlen(placeholder);
    if (strlen(output_out) + strlen(suffix) < max_len) {
        strcat(output_out, suffix);
    }
    
    return 0;
}

int tsfi_cw_niu_audit_copybook(const tsfi_cw_niu_copybook_field *fields, int field_count, int *total_bytes_out) {
    if (!fields || field_count < 0 || !total_bytes_out) return -1;
    
    *total_bytes_out = 0;
    for (int i = 0; i < field_count; i++) {
        *total_bytes_out += fields[i].size_bytes;
    }
    return 0;
}

int tsfi_cw_niu_audit_jcl_dd(const char *dd_statement, int *is_valid_path_out) {
    if (!dd_statement || !is_valid_path_out) return -1;
    
    *is_valid_path_out = 0;
    const char *dsn = strstr(dd_statement, "DSN=");
    if (!dsn) dsn = strstr(dd_statement, "DSNAME=");
    if (!dsn) return 0;
    
    // Skip key
    if (strncmp(dsn, "DSN=", 4) == 0) dsn += 4;
    else dsn += 7;
    
    // Validate characters: A-Z, 0-9, @, #, $, .
    int seg_len = 0;
    int has_seg = 0;
    while (*dsn && *dsn != ',' && *dsn != ' ' && *dsn != '\n' && *dsn != '\r') {
        char c = *dsn;
        if (c == '.') {
            if (seg_len == 0 || seg_len > 8) return 0;
            seg_len = 0;
        } else {
            // First char of segment must be letter or @, #, $
            if (seg_len == 0) {
                if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '@' || c == '#' || c == '$')) {
                    return 0;
                }
            } else {
                if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '@' || c == '#' || c == '$')) {
                    return 0;
                }
            }
            seg_len++;
            has_seg = 1;
        }
        dsn++;
    }
    
    if (seg_len == 0 || seg_len > 8 || !has_seg) return 0;
    
    *is_valid_path_out = 1;
    return 0;
}

int tsfi_cw_niu_audit_working_storage(const tsfi_cw_niu_cobol_var *vars, int var_count, int *uninitialized_count_out) {
    if (!vars || var_count < 0 || !uninitialized_count_out) return -1;
    
    *uninitialized_count_out = 0;
    for (int i = 0; i < var_count; i++) {
        if (!vars[i].has_value_clause) {
            (*uninitialized_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_rmu_audit_cics_security(const char *transaction_id, const char *user_security_key, int *is_authorized_out) {
    if (!transaction_id || !user_security_key || !is_authorized_out) return -1;
    
    *is_authorized_out = (strncmp(user_security_key, "SEC_", 4) == 0 || strcmp(user_security_key, "ADMIN") == 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_leap_defense_audit(const tsfi_cw_isu_leap_app *app, int defcon_level, const char *transaction_id, const char *user_security_key, double *criticality_risk_out) {
    if (!app || !criticality_risk_out) return -1;
    
    int auth = 0;
    tsfi_cw_rmu_audit_cics_security(transaction_id, user_security_key, &auth);
    
    double base = (app->database_size_gb * 0.05) + (app->transaction_rate_per_sec * 0.1);
    double nato_factor = (5.0 - app->nato_irs_compliance_rating) * 15.0;
    double mult = 1.0;
    
    switch (defcon_level) {
        case 1: mult = 3.0; break;
        case 2: mult = 2.0; break;
        case 3: mult = 1.5; break;
        case 4: mult = 1.2; break;
        case 5: default: mult = 1.0; break;
    }
    
    *criticality_risk_out = (base + nato_factor) * mult;
    if (!auth) {
        *criticality_risk_out += 500.0; // massive penalty for unauthorized transaction
    }
    return 0;
}

int tsfi_cw_isu_ulid_ssa_match(const char *ulid, const char *ssn_last4, const char *transaction_id, const char *user_security_key, int *is_match_out) {
    if (!ulid || !ssn_last4 || !is_match_out) return -1;
    if (strlen(ssn_last4) != 4) return -2;
    
    int auth = 0;
    tsfi_cw_rmu_audit_cics_security(transaction_id, user_security_key, &auth);
    if (!auth) {
        *is_match_out = 0;
        return -3; // security block
    }
    
    size_t ulid_len = strlen(ulid);
    if (ulid_len == 0) {
        *is_match_out = 0;
        return 0;
    }
    
    *is_match_out = (ulid[ulid_len - 1] == ssn_last4[3]) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_email(const tsfi_cw_isu_email_log *log, int *is_valid_out) {
    if (!log || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    const char *at = strchr(log->recipient_addr, '@');
    if (!at) return 0;
    if (strcmp(at, "@ilstu.edu") != 0 && strcmp(at, "@rs6000.cmp.ilstu.edu") != 0) return 0;
    
    if (log->is_mf_crossover) {
        // Enforce Micro Focus bounds: message size <= 64KB and non-empty sender
        size_t ulid_len = strlen(log->sender_ulid);
        if (log->message_bytes > 65536 || ulid_len < 2 || ulid_len > 8) {
            return 0;
        }
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_cw_isu_audit_sf_sla(const tsfi_cw_isu_state_farm_sla *sla, int *is_compliant_out) {
    if (!sla || !is_compliant_out) return -1;
    
    int limit = sla->max_allowed_seconds;
    if (sla->micro_focus_compat_flags & 0x01) {
        // Strict Mode: Max allowed is reduced by 20%
        limit = (int)(limit * 0.8);
    }
    
    *is_compliant_out = (sla->elapsed_seconds <= limit) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_enrollment(const tsfi_cw_isu_course_enrollment *enroll, int *can_register_out) {
    if (!enroll || !can_register_out) return -1;
    
    *can_register_out = (enroll->current_enrolled < enroll->max_capacity && enroll->prereqs_met_count == enroll->total_applicants) ? 1 : 0;
    return 0;
}

int tsfi_cw_isu_audit_gateway(const tsfi_cw_isu_gateway_status *gw, int *alert_out) {
    if (!gw || !alert_out) return -1;
    
    double drop_rate = (gw->packets_sent > 0) ? (double)gw->packets_dropped / (double)gw->packets_sent : 0.0;
    *alert_out = (drop_rate > 0.05 || gw->rtt_ms > 250) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_score_contestant(const tsfi_cw_unt_mtm_contestant *student, int *score_out) {
    if (!student || !score_out) return -1;
    
    int score = 0;
    if (student->part1_completed) score += 10;
    if (student->part2_completed) score += 30;
    score += student->part3_completed_tasks * 20;
    if (student->vsam_modified) score += 10;
    score -= student->jcl_errors * 15;
    
    *score_out = (score < 0) ? 0 : score;
    return 0;
}

int tsfi_cw_unt_audit_pipeline(const tsfi_cw_unt_crypto_pipeline *pipe, int *is_compliant_out) {
    if (!pipe || !is_compliant_out) return -1;
    
    *is_compliant_out = (strcmp(pipe->encryption_protocol, "AES-256") == 0 && pipe->unencrypted_fields_count == 0 && pipe->checksum_verified == 1) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_optimize_vsam(const tsfi_cw_unt_vsam_ksds *ksds, int *recommendation_flags_out) {
    if (!ksds || !recommendation_flags_out) return -1;
    
    *recommendation_flags_out = 0;
    if (ksds->control_interval_splits > 50) {
        *recommendation_flags_out |= 0x01;
    }
    if (ksds->buffer_space_allocated_kb < 128) {
        *recommendation_flags_out |= 0x02;
    }
    return 0;
}

int tsfi_cw_unt_audit_racf(const tsfi_cw_unt_racf_log *logs, int log_count, int *violations_out) {
    if (!logs || log_count < 0 || !violations_out) return -1;
    
    *violations_out = 0;
    for (int i = 0; i < log_count; i++) {
        if (!logs[i].is_authorized) {
            (*violations_out)++;
        }
    }
    return 0;
}

int tsfi_cw_raf_allocate_vital(const tsfi_cw_raf_vital_request *req, int *approved_out, int *priority_score_out) {
    if (!req || !approved_out || !priority_score_out) return -1;
    
    *approved_out = (req->stock_available >= req->qty_requested) ? 1 : 0;
    *priority_score_out = 0;
    if (strcmp(req->priority_level, "AOG") == 0) {
        *priority_score_out = 100;
    } else if (strcmp(req->priority_level, "ROUT") == 0) {
        *priority_score_out = 10;
    }
    return 0;
}

int tsfi_cw_raf_audit_ridels(const tsfi_cw_raf_ridels_audit *items, int item_count, double *total_discrepancy_value_out) {
    if (!items || item_count < 0 || !total_discrepancy_value_out) return -1;
    
    *total_discrepancy_value_out = 0.0;
    for (int i = 0; i < item_count; i++) {
        int diff = items[i].physical_count - items[i].system_count;
        if (diff < 0) diff = -diff;
        *total_discrepancy_value_out += diff * items[i].unit_cost;
    }
    return 0;
}

int tsfi_cw_unt_audit_pds(const tsfi_cw_unt_pds_status *pds, int *needs_compress_out) {
    if (!pds || !needs_compress_out) return -1;
    
    *needs_compress_out = (pds->total_directory_blocks > 0 && (double)pds->used_directory_blocks / (double)pds->total_directory_blocks >= 0.9) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_profile_cics(const tsfi_cw_unt_cics_tran *trans, int tran_count, double *avg_response_time_out, int *slow_count_out) {
    if (!trans || tran_count <= 0 || !avg_response_time_out || !slow_count_out) return -1;
    
    double total_res = 0.0;
    *slow_count_out = 0;
    for (int i = 0; i < tran_count; i++) {
        total_res += trans[i].response_time_ms;
        if (trans[i].response_time_ms > 1000) {
            (*slow_count_out)++;
        }
    }
    *avg_response_time_out = total_res / tran_count;
    return 0;
}

int tsfi_cw_unt_cics_audit_session(const tsfi_cw_unt_cics_session *session, int *needs_reset_out) {
    if (!session || !needs_reset_out) return -1;
    
    *needs_reset_out = (session->is_active && session->latency_ms > 2000) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_cics_audit_queue(const tsfi_cw_unt_cics_queue *queue, int *alert_out) {
    if (!queue || !alert_out) return -1;
    
    *alert_out = (queue->total_bytes > 50000000 || queue->item_count > 10000) ? 1 : 0;
    return 0;
}

int tsfi_cw_unt_cics_inject_ballistic(const char *data, int data_len, tsfi_cw_unt_cics_queue *queue, int *processed_bytes_out) {
    if (!data || data_len <= 0 || !queue || !processed_bytes_out) return -1;
    
    queue->item_count++;
    queue->total_bytes += data_len;
    *processed_bytes_out = data_len;
    return 0;
}

int g_ims_violations = 0;

int tsfi_cw_rmu_audit_ims(const tsfi_cw_rmu_ims_segment *segments, int segment_count, int *invalid_pointers_out) {
    if (!segments || segment_count < 0 || !invalid_pointers_out) return -1;
    
    *invalid_pointers_out = 0;
    for (int i = 0; i < segment_count; i++) {
        if (segments[i].pointer_address == 0 || !segments[i].is_valid) {
            (*invalid_pointers_out)++;
            continue;
        }
        if (strcmp(segments[i].parent_segment_name, "ROOT") != 0) {
            int found = 0;
            for (int j = 0; j < segment_count; j++) {
                if (strcmp(segments[j].segment_name, segments[i].parent_segment_name) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                (*invalid_pointers_out)++;
            }
        }
    }
    g_ims_violations = *invalid_pointers_out;
    return 0;
}

int tsfi_cw_rmu_audit_cics_web_gateway(const tsfi_cw_rmu_cics_web_request *req, int *is_compliant_out) {
    if (!req || !is_compliant_out) return -1;
    
    *is_compliant_out = (req->is_authorized && req->payload_bytes <= 1048576 && strcmp(req->header_content_type, "application/json") == 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_chase_audit_micr(const tsfi_cw_chase_micr_check *check, int *is_valid_out) {
    if (!check || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    if (strlen(check->routing_number) != 9) return 0;
    if (check->amount_dollars <= 0.0) return 0;
    
    int d[9];
    for (int i = 0; i < 9; i++) {
        if (check->routing_number[i] < '0' || check->routing_number[i] > '9') return 0;
        d[i] = check->routing_number[i] - '0';
    }
    
    int sum = 3 * (d[0] + d[3] + d[6]) + 7 * (d[1] + d[4] + d[7]) + (d[2] + d[5] + d[8]);
    if (sum % 10 == 0) {
        *is_valid_out = 1;
    }
    return 0;
}

int tsfi_cw_chase_audit_atm(const tsfi_cw_chase_atm_transaction *tx, int *is_valid_out) {
    if (!tx || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    if (strlen(tx->pin) != 4) return 0;
    for (int i = 0; i < 4; i++) {
        if (tx->pin[i] < '0' || tx->pin[i] > '9') return 0;
    }
    if (strlen(tx->card_number) < 12) return 0;
    
    // Hash/convert card number to key
    uint64_t card_key = 0;
    for (int i = 0; tx->card_number[i] != '\0'; i++) {
        if (tx->card_number[i] >= '0' && tx->card_number[i] <= '9') {
            card_key = card_key * 10 + (tx->card_number[i] - '0');
        }
    }
    
    // Attempt to load from evm_storage.dat.bin
    typedef struct {
        uint64_t key[4];
        uint64_t val[4];
        uint64_t addr;
    } EvmStorageRowAtm;
    
    static EvmStorageRowAtm rows[4096];
    uint32_t row_count = 0;
    
    const char *path = "evm_storage.dat.bin";
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        path = "tsfi2-deepseek/evm_storage.dat.bin";
        fp = fopen(path, "rb");
    }
    
    if (fp) {
        if (fread(&row_count, sizeof(row_count), 1, fp) == 1) {
            if (row_count > 4096) row_count = 4096;
            size_t read_bytes = fread(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)read_bytes;
        }
        fclose(fp);
    }
    
    // Find or create card entry in btc_rails_vm storage (address 0x9419441970718915ULL)
    uint64_t btc_rails_addr = 0x9419441970718915ULL;
    int found_index = -1;
    for (uint32_t i = 0; i < row_count; i++) {
        if (rows[i].addr == btc_rails_addr && rows[i].key[0] == card_key) {
            found_index = (int)i;
            break;
        }
    }
    
    double current_balance = 0.0;
    if (found_index != -1) {
        current_balance = (double)rows[found_index].val[0];
    } else {
        // Issue the card with a default balance of $5000 on btc rails ledger
        if (row_count < 4096) {
            found_index = (int)row_count;
            rows[found_index].addr = btc_rails_addr;
            rows[found_index].key[0] = card_key;
            rows[found_index].key[1] = 0;
            rows[found_index].key[2] = 0;
            rows[found_index].key[3] = 0;
            rows[found_index].val[0] = 5000; // $5000 USD
            rows[found_index].val[1] = 0;
            rows[found_index].val[2] = 0;
            rows[found_index].val[3] = 0;
            row_count++;
            current_balance = 5000.0;
        }
    }
    
    // Validate withdrawal
    if (tx->amount_dollars < 0.0 || tx->amount_dollars > current_balance) {
        return 0;
    }
    
    // Deduct funds
    if (found_index != -1) {
        rows[found_index].val[0] = (uint64_t)(current_balance - tx->amount_dollars);
        
        // Write back to database
        fp = fopen(path, "wb");
        if (fp) {
            fwrite(&row_count, sizeof(row_count), 1, fp);
            size_t written_bytes = fwrite(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)written_bytes;
            fclose(fp);
        }
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_cw_chase_issue_card(const char *card_number, const char *pin, double starting_balance, int *success_out) {
    if (!card_number || !pin || !success_out) return -1;
    
    *success_out = 0;
    if (strlen(pin) != 4) return 0;
    if (strlen(card_number) < 12) return 0;
    
    // Hash/convert card number to key
    uint64_t card_key = 0;
    for (int i = 0; card_number[i] != '\0'; i++) {
        if (card_number[i] >= '0' && card_number[i] <= '9') {
            card_key = card_key * 10 + (card_number[i] - '0');
        }
    }
    
    // Attempt to load from evm_storage.dat.bin
    typedef struct {
        uint64_t key[4];
        uint64_t val[4];
        uint64_t addr;
    } EvmStorageRowAtm;
    
    static EvmStorageRowAtm rows[4096];
    uint32_t row_count = 0;
    
    const char *path = "evm_storage.dat.bin";
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        path = "tsfi2-deepseek/evm_storage.dat.bin";
        fp = fopen(path, "rb");
    }
    
    if (fp) {
        if (fread(&row_count, sizeof(row_count), 1, fp) == 1) {
            if (row_count > 4096) row_count = 4096;
            size_t read_bytes = fread(rows, sizeof(EvmStorageRowAtm), row_count, fp);
            (void)read_bytes;
        }
        fclose(fp);
    }
    
    // Find or create card entry in btc_rails_vm storage (address 0x9419441970718915ULL)
    uint64_t btc_rails_addr = 0x9419441970718915ULL;
    int found_index = -1;
    for (uint32_t i = 0; i < row_count; i++) {
        if (rows[i].addr == btc_rails_addr && rows[i].key[0] == card_key) {
            found_index = (int)i;
            break;
        }
    }
    
    if (found_index != -1) {
        // Update balance
        rows[found_index].val[0] = (uint64_t)starting_balance;
    } else {
        // Create card row
        if (row_count < 4096) {
            found_index = (int)row_count;
            rows[found_index].addr = btc_rails_addr;
            rows[found_index].key[0] = card_key;
            rows[found_index].key[1] = 0;
            rows[found_index].key[2] = 0;
            rows[found_index].key[3] = 0;
            rows[found_index].val[0] = (uint64_t)starting_balance;
            rows[found_index].val[1] = 0;
            rows[found_index].val[2] = 0;
            rows[found_index].val[3] = 0;
            row_count++;
        }
    }
    
    // Write back to database
    fp = fopen(path, "wb");
    if (fp) {
        fwrite(&row_count, sizeof(row_count), 1, fp);
        size_t written_bytes = fwrite(rows, sizeof(EvmStorageRowAtm), row_count, fp);
        (void)written_bytes;
        fclose(fp);
        *success_out = 1;
    }
    
    return 0;
}











































