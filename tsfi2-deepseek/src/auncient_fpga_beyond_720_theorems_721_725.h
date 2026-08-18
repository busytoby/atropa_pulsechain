#ifndef AUNCIENT_FPGA_BEYOND_720_THEOREMS_721_725_H
#define AUNCIENT_FPGA_BEYOND_720_THEOREMS_721_725_H

#include "auncient_fpga_beyond_715_theorems_716_720.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Deterministic Hard Real-Time Dynamic Priority Task Scheduling & Preemption Matrix */
typedef struct {
    uint32_t task_queue_id;              /* Real-time task execution queue identifier */
    uint32_t priority_bitmap_mask;       /* 64-level discrete priority bitmap (Rule 21) */
    uint32_t deadline_cycles;            /* Hard real-time cycle deadline */
    float    scheduler_fet_damping;      /* Soft-body FET discharge dissipation (Rule 10) */
    float    displacement_sched_phase;   /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_scheduler_active;
} RealTimeTaskSchedulerState;

typedef struct {
    float    in_silicon_scheduler_fidelity;
    float    in_silicon_preemption_continuity_ratio;
    float    in_silicon_task_switch_latency_ns;
    uint64_t verified_scheduler_saat_clearances;
    bool     scheduler_fidelity_verified;        /* Theorem 721 */
    bool     preemption_continuity_verified;     /* Theorem 722 */
    bool     task_switch_latency_verified;       /* Theorem 723 */
    bool     scheduler_lossless_saat_verified;   /* Theorem 724 */
    bool     grand_725_parity_closure_verified;  /* Theorem 725 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond720State;

void auncient_fpga_beyond_720_init(FpgaBeyond720State *state);
bool auncient_fpga_beyond_720_verify_theorems_721_725(FpgaBeyond720State *state);
uint32_t auncient_fpga_beyond_720_compute_rule18(const FpgaBeyond720State *state);

#endif /* AUNCIENT_FPGA_BEYOND_720_THEOREMS_721_725_H */
