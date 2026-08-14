#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_cordis_composability.h"

int main(void) {
    printf("=== TESTING CORDIS SPATIOTEMPORAL COMPOSABILITY ENGINE ===\n");

    // 1. Test Static Prefix Pinning & Cache Hit
    tsfi_cordis_prefix_cache_t cache;
    assert(tsfi_cordis_prefix_cache_init(&cache));

    const char *sys_prompt = "Auncient VM System Register Map: Base, Channel, Signal, MotzkinPrime, Pole, Foundation, Dynamo.";
    assert(tsfi_cordis_prefix_cache_pin(&cache, sys_prompt, 18));
    assert(cache.pinned_token_count == 18);

    bool hit = false;
    assert(tsfi_cordis_prefix_cache_lookup(&cache, sys_prompt, &hit));
    assert(hit == true);
    printf("[1/3] Static prefix cache pinning & cache hit verified (Hit Count: %lu)\n", cache.cache_hit_count);

    // 2. Test Multi-Profile Lifecycle Initialization
    tsfi_cordis_composability_engine_t engine;
    assert(tsfi_cordis_composability_init(&engine, CORDIS_PROFILE_CODE));
    assert(strcmp(engine.active_profile.profile_name, "Code-Mode") == 0);
    assert(engine.active_profile.enable_stanag_coaxial == true);
    printf("[2/3] Multi-Profile engine initialized with Profile='Code-Mode'\n");

    // 3. Test 4-Stage Composition Cycle (Plan -> Speculative Pre-Exec -> Firewall Validation -> Commit)
    tsfi_edsac_acp_firewall_state_t fw;
    assert(tsfi_edsac_acp_firewall_init(&fw));

    char result_buf[512] = {0};
    bool res = tsfi_cordis_composability_execute_cycle(
        &engine,
        &fw,
        "Compile and link SCSI Yul virtual register handshake",
        "read_register",
        "reg=0x32",
        result_buf,
        sizeof(result_buf)
    );
    assert(res == true);
    assert(engine.stage_history_count == 4);
    assert(strstr(result_buf, "[CORDIS COMMIT]") != NULL);
    printf("[3/3] 4-Stage Composition cycle verified: %s\n", result_buf);

    printf("=== ALL CORDIS SPATIOTEMPORAL COMPOSABILITY TESTS PASSED ===\n");
    return 0;
}
