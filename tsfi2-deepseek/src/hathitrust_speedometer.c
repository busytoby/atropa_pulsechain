#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 199309L

#include "hathitrust_speedometer.h"
#include <stdio.h>

void hathitrust_speedometer_init(HtrcSpeedometer *speed) {
    if (!speed) return;
    clock_gettime(CLOCK_MONOTONIC, &speed->start_time);
    speed->last_update_time = speed->start_time;
    speed->total_bytes = 0;
    speed->last_bytes = 0;
}

void hathitrust_speedometer_update(HtrcSpeedometer *speed, size_t bytes_chunk, size_t expected_total) {
    if (!speed) return;
    
    speed->total_bytes += bytes_chunk;
    speed->last_bytes += bytes_chunk;
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    double elapsed = (now.tv_sec - speed->start_time.tv_sec) + 
                     (now.tv_nsec - speed->start_time.tv_nsec) / 1000000000.0;
                     
    double since_last = (now.tv_sec - speed->last_update_time.tv_sec) + 
                        (now.tv_nsec - speed->last_update_time.tv_nsec) / 1000000000.0;
                        
    // Print stats every 0.5 seconds or on completion
    if (since_last >= 0.5 || (expected_total > 0 && speed->total_bytes >= expected_total)) {
        double current_speed = (speed->last_bytes / 1024.0) / since_last;
        double avg_speed = (speed->total_bytes / 1024.0) / elapsed;
        
        if (expected_total > 0) {
            double percent = (double)speed->total_bytes / expected_total * 100.0;
            double eta = (percent > 0.0) ? (elapsed / percent * (100.0 - percent)) : 0.0;
            printf("[Harvest] Downloaded %zu/%zu bytes (%.1f%%) | Speed: %.1f KB/s | Avg: %.1f KB/s | ETA: %.1fs\n",
                   speed->total_bytes, expected_total, percent, current_speed, avg_speed, eta);
        } else {
            printf("[Harvest] Downloaded %zu bytes | Speed: %.1f KB/s | Avg: %.1f KB/s | Elapsed: %.1fs\n",
                   speed->total_bytes, current_speed, avg_speed, elapsed);
        }
        
        speed->last_update_time = now;
        speed->last_bytes = 0;
    }
}
