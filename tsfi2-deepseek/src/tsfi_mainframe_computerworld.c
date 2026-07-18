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



