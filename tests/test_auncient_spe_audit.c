#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint32_t total_ticks;
    uint32_t active_ticks; // Ticks where the system was fully operational
    uint32_t successful_reads;
    uint32_t failed_reads;
    double max_capacity; // Peak processing capability metric
} spe_tracker_t;

// Calculate and print the Systems Performance Effectiveness report
void compile_spe_report(const spe_tracker_t *tracker) {
    // 1. Availability = Active Ticks / Total Ticks
    double availability = (tracker->total_ticks > 0) ? 
        (double)tracker->active_ticks / tracker->total_ticks : 0.0;
        
    // 2. Dependability = Successful Reads / Total Reads
    uint32_t total_reads = tracker->successful_reads + tracker->failed_reads;
    double dependability = (total_reads > 0) ? 
        (double)tracker->successful_reads / total_reads : 1.0;
        
    // 3. Capability = Max Capacity coefficient (design constraint)
    double capability = tracker->max_capacity;

    // SPE = Capability * Availability * Dependability
    double spe = capability * availability * dependability;

    printf("   [AUDIT] Systems Performance Effectiveness Report (L. D. Whitelock SPE):\n");
    printf("      - Capability (C)   : %.4f\n", capability);
    printf("      - Availability (A) : %.4f (Uptime: %u/%u ticks)\n", 
           availability, tracker->active_ticks, tracker->total_ticks);
    printf("      - Dependability (D): %.4f (Reads: %u Succeeded, %u Failed)\n", 
           dependability, tracker->successful_reads, tracker->failed_reads);
    printf("      => SPE Coefficient  : %.4f (Target: >= 0.8500)\n", spe);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SYSTEMS PERFORMANCE EFFECTIVENESS AUDIT\n");
    printf("=============================================================\n");

    spe_tracker_t tracker = {
        .total_ticks = 0,
        .active_ticks = 0,
        .successful_reads = 0,
        .failed_reads = 0,
        .max_capacity = 0.95 // 95% performance ceiling under noise
    };

    // Simulate 100 ticks of system operation
    for (int tick = 1; tick <= 100; tick++) {
        tracker.total_ticks++;
        
        // Simulating normal operation (90% uptime)
        bool system_running = (tick % 10 != 0); // Fails every 10th tick (maintenance window)
        if (system_running) {
            tracker.active_ticks++;
        }

        // Simulate reads: 95% of reads succeed, 5% encounter parity decay faults
        for (int r = 0; r < 5; r++) {
            bool read_success = (rand() % 100 >= 5);
            if (read_success) {
                tracker.successful_reads++;
            } else {
                tracker.failed_reads++;
            }
        }
    }

    compile_spe_report(&tracker);

    // Verify metrics are populated
    assert(tracker.total_ticks == 100);
    assert(tracker.active_ticks == 90);
    
    printf("=============================================================\n");
    printf("SPE PERFORMANCE AUDIT COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
