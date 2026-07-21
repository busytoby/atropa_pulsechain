#ifndef TSFI_LYNCH_EXEC_H
#define TSFI_LYNCH_EXEC_H

#include <stdint.h>
#include <stddef.h>

#define LYNCH_MAX_PROCESSES 64

typedef enum {
    LYNCH_PROC_READY   = 0,
    LYNCH_PROC_RUNNING = 1,
    LYNCH_PROC_BLOCKED = 2,
    LYNCH_PROC_ZOMBIE  = 3
} tsfi_lynch_proc_state_t;

typedef struct {
    uint32_t pid;
    uint8_t priority;
    tsfi_lynch_proc_state_t state;
    uint32_t time_slice_ms;
    uint64_t working_set_pages;
} tsfi_lynch_pcb_t;

typedef struct {
    tsfi_lynch_pcb_t processes[LYNCH_MAX_PROCESSES];
    uint32_t process_count;
    uint32_t active_pid;
} tsfi_lynch_exec_t;

/* Initialize William C. Lynch's Executive Kernel and process scheduler */
int tsfi_lynch_exec_init(tsfi_lynch_exec_t *exec);

/* Spawn a process in Lynch's Executive Kernel process table */
int tsfi_lynch_exec_spawn(tsfi_lynch_exec_t *exec, uint8_t priority, uint32_t time_slice_ms, uint32_t *out_pid);

/* Schedule next preemptive process context switch according to Lynch working-set priority */
int tsfi_lynch_exec_schedule_next(tsfi_lynch_exec_t *exec, uint32_t *out_active_pid);

#endif // TSFI_LYNCH_EXEC_H
