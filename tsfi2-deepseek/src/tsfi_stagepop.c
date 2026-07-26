#include "tsfi_stagepop.h"
#include <string.h>

static void dfs_traverse(TSFiPrimNode *node, const char *type_name, TSFiStageTraversalResult *result) {
    if (!node || !type_name || !result) return;
    
    // Check if the current node matches the query filter
    if (strcmp(node->type_name, type_name) == 0) {
        if (result->match_count < TSFI_MAX_TRAVERSED_PRIMS) {
            result->matched_prims[result->match_count] = node;
            result->match_count++;
        }
    }
    
    // Recurse into children
    for (int i = 0; i < node->child_count; i++) {
        dfs_traverse(node->children[i], type_name, result);
    }
}

void tsfi_stagepop_traverse_type(TSFiPrimNode *root, const char *type_name, TSFiStageTraversalResult *result) {
    if (!result) return;
    result->match_count = 0;
    memset(result->matched_prims, 0, sizeof(result->matched_prims));
    
    dfs_traverse(root, type_name, result);
}
