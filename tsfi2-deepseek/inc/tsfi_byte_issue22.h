#ifndef TSFI_BYTE_ISSUE22_H
#define TSFI_BYTE_ISSUE22_H

#include <stdint.h>
#include <stdbool.h>

#define VECTOR_SIZE 8
#define DRAM_ROWS 8
#define DRAM_COLS 8

// APL Vector Executor Context
typedef struct {
    float data[VECTOR_SIZE];
} TsfiAplVector;

// Newt Cognitive Robot State
typedef struct {
    float x;
    float y;
    float target_x;
    float target_y;
    bool bumper_triggered;
    int col_count;
} TsfiNewtRobot;

// DRAM Cell Charge State (refresh modeling)
typedef struct {
    float charge[DRAM_ROWS][DRAM_COLS]; // Charge from 0.0f (0) to 1.0f (1)
    uint64_t last_refresh_us[DRAM_ROWS];
} TsfiDramConsole;

// --- API Declarations ---

// APL Vector operations
TsfiAplVector tsfi_apl_monadic_negate(const TsfiAplVector *v);
float tsfi_apl_monadic_sum(const TsfiAplVector *v);
TsfiAplVector tsfi_apl_dyadic_add(const TsfiAplVector *v1, const TsfiAplVector *v2);
TsfiAplVector tsfi_apl_dyadic_mul(const TsfiAplVector *v1, const TsfiAplVector *v2);

// Newt Pathfinding
void tsfi_newt_init(TsfiNewtRobot *robot, float start_x, float start_y, float target_x, float target_y);
void tsfi_newt_step(TsfiNewtRobot *robot, float obstacle_x, float obstacle_y, float obstacle_r);

// DRAM Console
void tsfi_dram_init(TsfiDramConsole *dram);
void tsfi_dram_write(TsfiDramConsole *dram, int row, int col, bool bit);
bool tsfi_dram_read(TsfiDramConsole *dram, int row, int col, uint64_t current_time_us);
void tsfi_dram_refresh_row(TsfiDramConsole *dram, int row, uint64_t current_time_us);
void tsfi_dram_decay_step(TsfiDramConsole *dram, uint64_t current_time_us, float decay_rate_per_us);

#endif // TSFI_BYTE_ISSUE22_H
