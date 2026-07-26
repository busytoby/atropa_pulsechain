#include "tsfi_reroute.h"
#include <string.h>
#include <stdio.h>

void tsfi_reroute_init(TSFiReroute *reroute) {
    if (!reroute) return;
    reroute->rule_count = 0;
    memset(reroute->rules, 0, sizeof(reroute->rules));
}

void tsfi_reroute_add_rule(TSFiReroute *reroute, const char *rel, const char *target, float dx, float dy, float dz) {
    if (!reroute || reroute->rule_count >= MAX_REROUTE_RULES) return;
    int idx = reroute->rule_count++;
    strncpy(reroute->rules[idx].relative_prefix, rel, MAX_PATH_LEN - 1);
    strncpy(reroute->rules[idx].target_prefix, target, MAX_PATH_LEN - 1);
    reroute->rules[idx].displacement[0] = dx;
    reroute->rules[idx].displacement[1] = dy;
    reroute->rules[idx].displacement[2] = dz;
}

bool tsfi_reroute_resolve(const TSFiReroute *reroute, const char *rel_path, char *out_abs_path, float *out_pos) {
    if (!reroute || !rel_path || !out_abs_path || !out_pos) return false;
    
    // Default fallback: copy path directly
    strcpy(out_abs_path, rel_path);
    out_pos[0] = 0.0f;
    out_pos[1] = 0.0f;
    out_pos[2] = 0.0f;

    for (int i = 0; i < reroute->rule_count; i++) {
        const RerouteRule *rule = &reroute->rules[i];
        size_t len = strlen(rule->relative_prefix);
        if (strncmp(rel_path, rule->relative_prefix, len) == 0) {
            // Found matching relative prefix -> Translate path via: Path_absolute = Prefix_target + Path_relative
            snprintf(out_abs_path, MAX_PATH_LEN, "%s%s", rule->target_prefix, rel_path + len);
            
            // Map visual translation coordinates
            out_pos[0] = rule->displacement[0];
            out_pos[1] = rule->displacement[1];
            out_pos[2] = rule->displacement[2];
            return true;
        }
    }
    return false;
}
