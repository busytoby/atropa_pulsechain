#include "tsfi_byte_issue22.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// APL Vector operations implementation
TsfiAplVector tsfi_apl_monadic_negate(const TsfiAplVector *v) {
    TsfiAplVector out;
    if (!v) return out;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        out.data[i] = -v->data[i];
    }
    return out;
}

float tsfi_apl_monadic_sum(const TsfiAplVector *v) {
    if (!v) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        sum += v->data[i];
    }
    return sum;
}

TsfiAplVector tsfi_apl_dyadic_add(const TsfiAplVector *v1, const TsfiAplVector *v2) {
    TsfiAplVector out;
    if (!v1 || !v2) return out;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        out.data[i] = v1->data[i] + v2->data[i];
    }
    return out;
}

TsfiAplVector tsfi_apl_dyadic_mul(const TsfiAplVector *v1, const TsfiAplVector *v2) {
    TsfiAplVector out;
    if (!v1 || !v2) return out;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        out.data[i] = v1->data[i] * v2->data[i];
    }
    return out;
}

// Newt Cognitive Robot implementation
void tsfi_newt_init(TsfiNewtRobot *robot, float start_x, float start_y, float target_x, float target_y) {
    if (!robot) return;
    robot->x = start_x;
    robot->y = start_y;
    robot->target_x = target_x;
    robot->target_y = target_y;
    robot->bumper_triggered = false;
    robot->col_count = 0;
}

void tsfi_newt_step(TsfiNewtRobot *robot, float obstacle_x, float obstacle_y, float obstacle_r) {
    if (!robot) return;

    robot->bumper_triggered = false;

    // Check collision reflex first
    float ox = robot->x - obstacle_x;
    float oy = robot->y - obstacle_y;
    float dist = sqrtf(ox * ox + oy * oy);

    if (dist < obstacle_r) {
        // Bumper triggered! Bounce back reflex.
        robot->bumper_triggered = true;
        robot->col_count++;
        
        // Push away from obstacle center
        if (dist > 1e-5f) {
            robot->x += (ox / dist) * 0.3f;
            robot->y += (oy / dist) * 0.3f;
        } else {
            robot->x += 0.3f;
        }
        return;
    }

    // Cognitive target navigation
    float tx = robot->target_x - robot->x;
    float ty = robot->target_y - robot->y;
    float t_dist = sqrtf(tx * tx + ty * ty);

    if (t_dist > 0.05f) {
        // Move towards target
        robot->x += (tx / t_dist) * 0.1f;
        robot->y += (ty / t_dist) * 0.1f;
    }
}

// DRAM Console implementation
void tsfi_dram_init(TsfiDramConsole *dram) {
    if (!dram) return;
    memset(dram, 0, sizeof(TsfiDramConsole));
}

void tsfi_dram_write(TsfiDramConsole *dram, int row, int col, bool bit) {
    if (!dram || row >= DRAM_ROWS || col >= DRAM_COLS) return;
    dram->charge[row][col] = bit ? 1.0f : 0.0f;
}

bool tsfi_dram_read(TsfiDramConsole *dram, int row, int col, uint64_t current_time_us) {
    if (!dram || row >= DRAM_ROWS || col >= DRAM_COLS) return false;

    // Simulate decay since last refresh/read
    uint64_t time_delta = current_time_us - dram->last_refresh_us[row];
    // Linear decay model: decays to 0.0V (logic 0) or degrades based on leakage
    float decay = (float)time_delta * 0.00025f; // Decay factor
    float current_charge = dram->charge[row][col];

    if (current_charge > 0.5f) {
        current_charge -= decay;
        if (current_charge < 0.0f) current_charge = 0.0f;
    } else {
        // Logic 0 charges decay towards 0 as well
        current_charge += decay * 0.1f; // minimal upward noise drift
        if (current_charge > 0.5f) current_charge = 0.5f;
    }

    // Dynamic refresh on read
    dram->charge[row][col] = (current_charge > 0.5f) ? 1.0f : 0.0f;
    dram->last_refresh_us[row] = current_time_us;

    return (current_charge > 0.5f);
}

void tsfi_dram_refresh_row(TsfiDramConsole *dram, int row, uint64_t current_time_us) {
    if (!dram || row >= DRAM_ROWS) return;

    // Refresh boosts cell charges back to their full logical levels
    for (int col = 0; col < DRAM_COLS; col++) {
        uint64_t time_delta = current_time_us - dram->last_refresh_us[row];
        float decay = (float)time_delta * 0.00025f;
        float current_charge = dram->charge[row][col] - decay;
        dram->charge[row][col] = (current_charge > 0.5f) ? 1.0f : 0.0f;
    }
    dram->last_refresh_us[row] = current_time_us;
}

void tsfi_dram_decay_step(TsfiDramConsole *dram, uint64_t current_time_us, float decay_rate_per_us) {
    if (!dram) return;
    for (int r = 0; r < DRAM_ROWS; r++) {
        uint64_t time_delta = current_time_us - dram->last_refresh_us[r];
        if (time_delta > 0) {
            for (int c = 0; c < DRAM_COLS; c++) {
                float current_charge = dram->charge[r][c];
                if (current_charge > 0.5f) {
                    current_charge -= decay_rate_per_us * (float)time_delta;
                    if (current_charge < 0.0f) current_charge = 0.0f;
                    dram->charge[r][c] = current_charge;
                }
            }
            dram->last_refresh_us[r] = current_time_us;
        }
    }
}
