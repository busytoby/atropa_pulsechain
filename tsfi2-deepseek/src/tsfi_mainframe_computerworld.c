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

















