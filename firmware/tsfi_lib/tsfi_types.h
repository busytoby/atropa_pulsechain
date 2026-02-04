#ifndef TSFI_TYPES_H
#define TSFI_TYPES_H

#include <stdbool.h>

// Common fields for mapped structs
#define MAPPED_COMMON_FIELDS \
    void (*step_safety_epoch)(void); \
    void (*step_safety_state)(void); \
    void (*step_executor_directive)(char*); \
    void (*scramble)(void); \
    void (*provenance)(void); \
    int * const version; \
    char ** const resonance_as_status; \
    bool * const ftw; \
    int * const counter; \
    bool * const is_autonomous_excuse_active;

typedef struct {
    MAPPED_COMMON_FIELDS
} MappedCommon;

// Hilbert Series Definition: Enumerates the dimension of graded components
// The structure layout defines the "series" of fields that map the WaveSystem topology.
// Flattened macro to avoid line continuation issues
#define DEFINE_MAPPED_STRUCT(name, ...) typedef struct { struct { MAPPED_COMMON_FIELDS }; __VA_ARGS__ } name;

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id; char *current_directive; double current_intensity; char *provenance_sig;)

#endif // TSFI_TYPES_H