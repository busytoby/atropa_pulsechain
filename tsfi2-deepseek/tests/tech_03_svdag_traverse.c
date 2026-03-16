#include <stdio.h>
#include <stdlib.h>
#include "tsfi_svdag.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi Tech Test: Helmholtz SVDAG Traversal (Repaired) ===\n");
    
    // Use the formal API
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(1024);
    if (!dag) return 1;

    printf("[AUDIT] SVDAG created. Capacity: %zu voxels.\n", dag->stream_capacity);
    
    // Execute the empty DAG (should return 0.0 mass)
    float mass = tsfi_svdag_execute(dag);
    printf("[AUDIT] Executed SVDAG. Resulting Mass: %.2f\n", mass);
    
    tsfi_svdag_destroy(dag);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
