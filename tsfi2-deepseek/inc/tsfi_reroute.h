#ifndef TSFI_REROUTE_H
#define TSFI_REROUTE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PATH_LEN 128
#define MAX_REROUTE_RULES 16

typedef struct {
    char relative_prefix[MAX_PATH_LEN];
    char target_prefix[MAX_PATH_LEN];
    float displacement[3]; // Translation coordinate mapping offsets
} RerouteRule;

// Namespace Re-router (Reroute) context
typedef struct {
    RerouteRule rules[MAX_REROUTE_RULES];
    int rule_count;
} TSFiReroute;

// Initialize the namespace re-router
void tsfi_reroute_init(TSFiReroute *reroute);

// Add a prefix routing rule to the namespace re-router
void tsfi_reroute_add_rule(TSFiReroute *reroute, const char *rel, const char *target, float dx, float dy, float dz);

// Transforms relative coordinate paths to absolute paths and applies translation mapping
bool tsfi_reroute_resolve(const TSFiReroute *reroute, const char *rel_path, char *out_abs_path, float *out_pos);

#endif // TSFI_REROUTE_H
