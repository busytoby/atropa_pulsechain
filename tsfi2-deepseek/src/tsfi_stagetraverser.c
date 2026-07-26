#include "tsfi_stagetraverser.h"
#include <string.h>

void tsfi_stagetraverser_init_predicate(TSFiTraversalPredicate *pred, bool include_inactive, const char *type_filter) {
    if (!pred) return;
    
    pred->include_inactive = include_inactive;
    if (type_filter) {
        strncpy(pred->type_filter, type_filter, sizeof(pred->type_filter) - 1);
    } else {
        memset(pred->type_filter, 0, sizeof(pred->type_filter));
    }
}

static void recursive_traverse(TSFiPrimNode *node, const TSFiTraversalPredicate *pred, TSFiTraversalRange *range) {
    if (!node || !pred || !range) return;
    
    // Check active filtering
    bool is_active = true;
    
    // Read prim node active flag from attribute if it exists, default true
    float active_val = tsfi_primroot_get_attribute(node, "active", 1.0f);
    if (active_val == 0.0f) {
        is_active = false;
    }
    
    // Prune branches if node is inactive and we don't include inactive nodes
    if (!is_active && !pred->include_inactive) {
        return;
    }
    
    // Check type matching criteria
    bool type_matches = true;
    if (strlen(pred->type_filter) > 0 && strcmp(node->type_name, pred->type_filter) != 0) {
        type_matches = false;
    }
    
    // Collect matching node
    if (type_matches) {
        if (range->count < TSFI_MAX_TRAVERSED_NODES) {
            range->nodes[range->count] = node;
            range->count++;
        }
    }
    
    // Traverse children
    for (int i = 0; i < node->child_count; i++) {
        recursive_traverse(node->children[i], pred, range);
    }
}

void tsfi_stagetraverser_traverse(TSFiPrimNode *root, const TSFiTraversalPredicate *pred, TSFiTraversalRange *range) {
    if (!range) return;
    range->count = 0;
    memset(range->nodes, 0, sizeof(range->nodes));
    
    recursive_traverse(root, pred, range);
}
