#include "tsfi_stagemask.h"
#include <string.h>

void tsfi_stagemask_init(TSFiStageMask *mask) {
    if (!mask) return;
    
    mask->prefix_count = 0;
    mask->is_enabled = false;
    memset(mask->allowed_prefixes, 0, sizeof(mask->allowed_prefixes));
}

bool tsfi_stagemask_add_prefix(TSFiStageMask *mask, const char *prefix) {
    if (!mask || !prefix || mask->prefix_count >= TSFI_MAX_MASK_PATHS) return false;
    
    // Check for duplicate prefixes
    for (int i = 0; i < mask->prefix_count; i++) {
        if (strcmp(mask->allowed_prefixes[i], prefix) == 0) {
            return true;
        }
    }
    
    strncpy(mask->allowed_prefixes[mask->prefix_count], prefix, sizeof(mask->allowed_prefixes[mask->prefix_count]) - 1);
    mask->prefix_count++;
    mask->is_enabled = true;
    return true;
}

bool tsfi_stagemask_is_path_allowed(const TSFiStageMask *mask, const char *path) {
    if (!mask || !path) return false;
    if (!mask->is_enabled) return true; // Mask disabled: all paths allowed
    
    // Check if the path matches any allowed prefix (prefix match)
    for (int i = 0; i < mask->prefix_count; i++) {
        const char *prefix = mask->allowed_prefixes[i];
        size_t len = strlen(prefix);
        
        if (strncmp(path, prefix, len) == 0) {
            // Verify path component boundaries (either matching exactly or followed by '/')
            if (path[len] == '\0' || path[len] == '/') {
                return true;
            }
        }
    }
    
    return false;
}
