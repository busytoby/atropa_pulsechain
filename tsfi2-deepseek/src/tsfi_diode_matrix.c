#include "tsfi_diode_matrix.h"
#include <string.h>
#include <math.h>

void tsfi_diode_matrix_init(TsfiDiodeMatrix *matrix) {
    memset(matrix, 0, sizeof(TsfiDiodeMatrix));
    matrix->v_cc = 5.0f;
    matrix->r_pullup = 2000.0f; // 2k
    matrix->v_f = 0.7f;         // 0.7V diode drop
    matrix->r_diode = 50.0f;    // 50 Ohm bulk resistance
    matrix->i_leak_0 = 1e-6f;   // 1 uA leakage at 25C
    matrix->temp = 25.0f;
    matrix->t = 0.0f;
}

int tsfi_diode_matrix_step(TsfiDiodeMatrix *matrix, uint8_t address, float dt_ns) {
    float dt_sec = dt_ns * 1e-9f;
    matrix->t += dt_sec;

    // Calculate temperature-dependent reverse leakage current
    // Leakage doubles every 10C
    float i_leak = matrix->i_leak_0 * powf(2.0f, (matrix->temp - 25.0f) / 10.0f);

    // High and Low driver voltage levels
    float v_low = 0.2f;
    float v_high = 3.0f;

    // Address input bits
    uint8_t a2 = (address >> 2) & 1;
    uint8_t a1 = (address >> 1) & 1;
    uint8_t a0 = address & 1;

    // True and complement line voltages
    float v_a[3] = {
        a0 ? v_high : v_low,
        a1 ? v_high : v_low,
        a2 ? v_high : v_low
    };
    float v_a_bar[3] = {
        a0 ? v_low : v_high,
        a1 ? v_low : v_high,
        a2 ? v_low : v_high
    };

    int active_select = -1;
    float max_v_out = -1.0f;

    for (int j = 0; j < 8; j++) {
        // Output code bits for line j
        uint8_t j2 = (j >> 2) & 1;
        uint8_t j1 = (j >> 1) & 1;
        uint8_t j0 = j & 1;

        // Determine the three input line voltages connected to line j's diodes
        float v_lines[3] = {
            j0 ? v_a[0] : v_a_bar[0],
            j1 ? v_a[1] : v_a_bar[1],
            j2 ? v_a[2] : v_a_bar[2]
        };

        // Determine if any connected line is LOW
        // If the address matches j, all v_lines are HIGH (3.0V).
        // If the address differs, at least one v_line is LOW (0.2V).
        float min_line_v = v_lines[0];
        int low_count = 0;
        int high_count = 0;

        for (int b = 0; b < 3; b++) {
            if (v_lines[b] < min_line_v) {
                min_line_v = v_lines[b];
            }
            if (v_lines[b] == v_low) {
                low_count++;
            } else {
                high_count++;
            }
        }

        // Calculate node voltage V_j
        float v_j = matrix->v_cc;

        if (low_count > 0) {
            // Conducting state: pull-down dominates
            // The equivalent circuit is pull-up to Vcc, diode conduction to min_line_v.
            // V_j = (V_cc/R_pullup + (min_line_v + V_f)/R_diode) / (1/R_pullup + 1/R_diode)
            // Plus leakage current from reverse biased diodes: - high_count * i_leak
            float g_pullup = 1.0f / matrix->r_pullup;
            float g_diode = 1.0f / matrix->r_diode;
            
            float num = matrix->v_cc * g_pullup + (min_line_v + matrix->v_f) * g_diode - high_count * i_leak;
            float den = g_pullup + g_diode;
            v_j = num / den;
            
            if (v_j < min_line_v + matrix->v_f) {
                v_j = min_line_v + matrix->v_f;
            }
        } else {
            // Selected state: all diodes are reverse biased.
            // Node voltage is pulled up to Vcc minus reverse leakage current of the 3 diodes:
            // V_j = V_cc - R_pullup * 3 * i_leak
            v_j = matrix->v_cc - matrix->r_pullup * 3.0f * i_leak;
        }

        if (v_j > matrix->v_cc) v_j = matrix->v_cc;
        if (v_j < 0.0f) v_j = 0.0f;

        matrix->v_outputs[j] = v_j;

        if (v_j > max_v_out) {
            max_v_out = v_j;
            active_select = j;
        }
    }

    return active_select;
}
