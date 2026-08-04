#ifndef HATHITRUST_SPEEDOMETER_H
#define HATHITRUST_SPEEDOMETER_H

#include <stddef.h>
#include <time.h>

typedef struct {
    struct timespec start_time;
    struct timespec last_update_time;
    size_t total_bytes;
    size_t last_bytes;
} HtrcSpeedometer;

/*
 * Initializes speedometer state.
 */
void hathitrust_speedometer_init(HtrcSpeedometer *speed);

/*
 * Updates speedometer with downloaded chunk size and prints progress.
 * Parameters:
 *   speed: Speedometer state.
 *   bytes_chunk: Bytes downloaded in last block.
 *   expected_total: Total expected download size (set to 0 if unknown).
 */
void hathitrust_speedometer_update(HtrcSpeedometer *speed, size_t bytes_chunk, size_t expected_total);

#endif /* HATHITRUST_SPEEDOMETER_H */
