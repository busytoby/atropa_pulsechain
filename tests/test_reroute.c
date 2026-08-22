#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_reroute.h"

int main(void) {
    printf("=== RUNNING AUNCIENT NAMESPACE RE-ROUTER (REROUTE) TESTS ===\n");

    TSFiReroute reroute;
    tsfi_reroute_init(&reroute);

    // Initial state
    assert(reroute.rule_count == 0);

    // Add a routing rule
    tsfi_reroute_add_rule(&reroute, "/local/bear/", "/root/assets/teddy_bear/", 10.0f, -5.0f, 3.5f);
    assert(reroute.rule_count == 1);

    char resolved_path[MAX_PATH_LEN];
    float resolved_pos[3];

    // 1. Test resolving matching path
    bool ok = tsfi_reroute_resolve(&reroute, "/local/bear/mesh.dat.bin", resolved_path, resolved_pos);
    assert(ok == true);
    printf("   Resolved Path: %s\n", resolved_path);
    assert(strcmp(resolved_path, "/root/assets/teddy_bear/mesh.dat.bin") == 0);
    assert(fabs(resolved_pos[0] - 10.0f) < 1e-5f);
    assert(fabs(resolved_pos[1] - (-5.0f)) < 1e-5f);
    assert(fabs(resolved_pos[2] - 3.5f) < 1e-5f);

    // 2. Test resolving non-matching path
    ok = tsfi_reroute_resolve(&reroute, "/other/path/mesh.dat.bin", resolved_path, resolved_pos);
    assert(ok == false);
    assert(strcmp(resolved_path, "/other/path/mesh.dat.bin") == 0);
    assert(resolved_pos[0] == 0.0f);

    printf("   ✓ Namespace translation and translation positioning verified successfully.\n");
    printf("=== AUNCIENT NAMESPACE RE-ROUTER (REROUTE) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
