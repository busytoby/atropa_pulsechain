#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lynch_exec.h"

int tsfi_lynch_exec_init(tsfi_lynch_exec_t *exec) {
    if (!exec) return -1;
    memset(exec, 0, sizeof(tsfi_lynch_exec_t));

    // Spawn Root Kernel Process (PID 1)
    exec->processes[0].pid = 1;
    exec->processes[0].priority = 0; // Highest Priority
    exec->processes[0].state = LYNCH_PROC_RUNNING;
    exec->processes[0].time_slice_ms = 10;
    exec->processes[0].working_set_pages = 64;

    exec->process_count = 1;
    exec->active_pid = 1;

    printf("[LYNCH KERNEL] Initialized William C. Lynch Executive Kernel (PID 1 Online)\n");
    return 0;
}

int tsfi_lynch_exec_spawn(tsfi_lynch_exec_t *exec, uint8_t priority, uint32_t time_slice_ms, uint32_t *out_pid) {
    if (!exec || exec->process_count >= LYNCH_MAX_PROCESSES) return -1;

    uint32_t idx = exec->process_count;
    uint32_t pid = idx + 1;

    exec->processes[idx].pid = pid;
    exec->processes[idx].priority = priority;
    exec->processes[idx].state = LYNCH_PROC_READY;
    exec->processes[idx].time_slice_ms = time_slice_ms;
    exec->processes[idx].working_set_pages = 16;

    exec->process_count++;
    if (out_pid) *out_pid = pid;

    printf("[LYNCH KERNEL] Spawned Process PID %u (Priority Tier: %u, TimeSlice: %u ms)\n",
           pid, priority, time_slice_ms);

    return 0;
}

int tsfi_lynch_exec_schedule_next(tsfi_lynch_exec_t *exec, uint32_t *out_active_pid) {
    if (!exec || exec->process_count == 0) return -1;

    uint32_t highest_prio_idx = 0;
    uint8_t min_prio = 255;

    for (uint32_t i = 0; i < exec->process_count; i++) {
        if (exec->processes[i].state == LYNCH_PROC_READY || exec->processes[i].state == LYNCH_PROC_RUNNING) {
            if (exec->processes[i].priority < min_prio) {
                min_prio = exec->processes[i].priority;
                highest_prio_idx = i;
            }
        }
    }

    exec->active_pid = exec->processes[highest_prio_idx].pid;
    exec->processes[highest_prio_idx].state = LYNCH_PROC_RUNNING;

    if (out_active_pid) *out_active_pid = exec->active_pid;

    printf("[LYNCH SCHEDULER] Context Switch -> Active Process PID %u (Priority %u)\n",
           exec->active_pid, min_prio);

    return 0;
}
