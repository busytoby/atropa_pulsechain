#include <stdint.h>
#include <stddef.h>
#include "tsfi_svdag.h"

/**
 * TSFi Ballet Step 7: Tourner (Turning)
 * SVDAG turns. Decision resolves. Secret is set.
 * This thunk executes a single step of the SVDAG decision process.
 */
float tsfi_ballet_tourner_thunk(TSFiSVDAG *svdag, uint32_t node_id) {
    if (!svdag) return 0.0f;
    
    // The SVDAG "turns" to resolve a specific decision path
    float value = tsfi_svdag_query(svdag, node_id);
    
    // Once the decision resolves, the secret is set.
    return value;
}
