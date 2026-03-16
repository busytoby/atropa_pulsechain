#ifndef LAU_REGISTRY_H
#define LAU_REGISTRY_H
extern _Atomic int g_init_in_progress;
extern _Atomic int g_teardown_in_progress;
void lau_registry_teardown(void);
void* lau_registry_get_local_manifold(void);

#include "lau_memory.h"
#include "lau_telemetry.h"

// Inserts a metadata block into the global tracking registry.
void lau_registry_insert(LauMetadata *m);

// Removes a metadata block from the global tracking registry.
void lau_registry_remove(LauMetadata *m);

// Searches the registry for metadata corresponding to a payload pointer.
LauMetadata* lau_registry_find(void *payload);
void lau_registry_report_event(uint64_t ptr, uint64_t size, uint32_t type, uint32_t flags);

// Returns the head of the registry list (requires locking via lau_registry_lock).
LauMetadata* lau_registry_get_head(void);
uint32_t lau_registry_get_count(void);

// High-speed ZMM-accelerated manifold scan.
LauMetadata* tsfi_registry_scan_zmm(LauRegistryManifold *m, void *ptr);

// Locking for registry traversal
void lau_registry_lock(void);
void lau_registry_unlock(void);

// Telemetry
void lau_telemetry_record_exec(const char *directive);
extern _Atomic int g_init_in_progress;
extern _Atomic int g_teardown_in_progress;
void lau_registry_teardown(void);
void* lau_registry_get_local_manifold(void);
LauTelemetryState* lau_telemetry_get_state(void);

#endif // LAU_REGISTRY_H
