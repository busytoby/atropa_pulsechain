#include "tsfi_broadcaster.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"
#include "lau_registry.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdatomic.h>

/**
 * @brief Strictly deterministic bandwidth throttling.
 * 
 * Logic: Calculates current throughput vs target_bitrate_bps.
 * Stalls execution if dispatch rate exceeds Phase 1 limits.
 */
void tsfi_broadcaster_throttle_bytes(LauBroadcaster *lb, size_t bytes) {
    if (!lb || lb->target_bitrate_bps == 0) return;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;

    // Initialize window if needed
    if (lb->window_start_ns == 0) {
        lb->window_start_ns = now_ns;
        lb->bytes_sent_window = 0;
    }

    lb->bytes_sent_window += bytes;

    uint64_t elapsed_ns = now_ns - lb->window_start_ns;
    if (elapsed_ns > 5000000000ULL) { // Increased window to 5s for smoother averaging
        lb->window_start_ns = now_ns;
        lb->bytes_sent_window = bytes;
        return;
    }

    // Calculate target bytes allowed for elapsed time
    uint64_t allowed_bytes = (lb->target_bitrate_bps / 8) * elapsed_ns / 1000000000ULL;

    if (lb->bytes_sent_window > allowed_bytes) {
        // Calculate required sleep to reach allowed rate
        uint64_t required_ns = (lb->bytes_sent_window * 8 * 1000000000ULL) / lb->target_bitrate_bps;
        uint64_t sleep_ns = required_ns - elapsed_ns;
        
        if (sleep_ns > 0 && sleep_ns < 5000000000ULL) {
            LauTelemetryState *telem = lau_telemetry_get_state();
            if (telem) {
                atomic_fetch_add(&telem->throttle_count, 1);
                atomic_fetch_add(&telem->total_stall_ns, sleep_ns);
            }
            
            tsfi_io_printf(stdout, "[THROTTLE] Sleeping %llu ns (Window: %llu bytes, Allowed: %llu bytes)\n", (unsigned long long)sleep_ns, (unsigned long long)lb->bytes_sent_window, (unsigned long long)allowed_bytes);
            tsfi_raw_usleep(sleep_ns / 1000);
        }
    }
}
