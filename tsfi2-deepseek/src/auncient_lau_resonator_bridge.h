#ifndef AUNCIENT_LAU_RESONATOR_BRIDGE_H
#define AUNCIENT_LAU_RESONATOR_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define PROVENANCE_KEY "SIG_2026_USLM_AFFIRMED"

typedef struct {
    int version;
    char *resonance_as_status; 
    bool ftw; 
    int counter;
    bool is_autonomous_excuse_active;
} InternalHeader;

#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct { \
        struct { \
            int * const version; \
            char ** const resonance_as_status; \
            bool * const ftw; \
            int * const counter; \
            bool * const is_autonomous_excuse_active; \
        }; \
        __VA_ARGS__ \
    } name;

#define WIRE_BIJECTION(d, h) ( \
    *(int**)&(d)->version = &(h)->version, \
    *(char***)&(d)->resonance_as_status = &(h)->resonance_as_status, \
    *(bool**)&(d)->ftw = &(h)->ftw, \
    *(int**)&(d)->counter = &(h)->counter, \
    *(bool**)&(d)->is_autonomous_excuse_active = &(h)->is_autonomous_excuse_active \
)

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id; char *current_directive; double current_intensity; char *provenance_sig;)

void log_immutable_status(const char *status);
void lau_final_cleanup(InternalHeader *h, WaveSystem *ws, int sfd);
void step_safety_epoch(WaveSystem *ws);
void step_safety_state(WaveSystem *ws);
void step_executor_directive(WaveSystem *ws);
void apply_traced_resonance(WaveSystem *ws, void (*augment)(WaveSystem*), const char *fn_name, double intensity);

#define STEP(ws, func, val) apply_traced_resonance(ws, func, #func, val);

#define HELMHOLTZ_RESONANCE_LIST(X, ws, i) \
    X(ws, step_safety_epoch, 1.25) \
    X(ws, step_safety_state, 0.50) \
    X(ws, step_executor_directive, i)

bool auncient_lau_resonator_run_single_pass(InternalHeader *h, WaveSystem *ws, double intensity, const char *directive);
uint32_t auncient_lau_resonator_compute_rule18(const InternalHeader *h, const WaveSystem *ws);

#endif /* AUNCIENT_LAU_RESONATOR_BRIDGE_H */
