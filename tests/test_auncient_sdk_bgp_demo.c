#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "auncient_sdk.h"

#define STATE_FILE "sdk_bgp_demo.dat.bin"

// BGP Route Advertisement Record
typedef struct {
    uint32_t prefix_space;
    uint32_t originating_as;
    uint32_t resolved_dynamic_address;
} bgp_sdk_route_t;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SDK BGP NETWORKING DEMO\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Initialize Coaxial Environment and SDK Context
    sdk_coaxial_env_t env;
    memset(&env, 0, sizeof(sdk_coaxial_env_t));
    bool init_ok = auncient_sdk_init_coaxial(&env);
    assert(init_ok == true);

    sdk_kermit_cache_t cache;
    memset(&cache, 0, sizeof(sdk_kermit_cache_t));

    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 3,
        .state = SDK_STATE_EXECUTING
    };

    printf("[SDK BGP] Coaxial environment and context initialized.\n");
    fflush(stdout);

    // 2. Mock dynamic BGP discovery routing events using the SDK
    bgp_sdk_route_t demo_route = {
        .prefix_space = 0x4000,
        .originating_as = 65001,
        .resolved_dynamic_address = 0x51E9A4B
    };

    // Execute mock write operation in SDK context representing BGP advertisement
    uint32_t result_val = 0;
    bool approvals[SDK_NUM_NODES] = {true, true, true, false};

    printf("[SDK BGP] Advertising route to prefix 0x%X from AS-%u using SDK...\n",
           demo_route.prefix_space, demo_route.originating_as);
    fflush(stdout);

    // Execute SDK contract step
    bool exec_ok = auncient_sdk_alu_execute(&ctx, ALU_OP_WRITE_ABD, 777, approvals, &result_val);
    assert(exec_ok == true);
    assert(result_val == 777);

    printf("   ✓ SDK route execution result: 0x%X\n", result_val);
    fflush(stdout);

    // 3. Serialize BGP Route to DAT database
    printf("[SDK BGP] Logging discovered route to DAT database %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(&demo_route, sizeof(bgp_sdk_route_t), 1, file);
    assert(written == 1);
    fclose(file);

    // Read back and verify
    printf("[SDK BGP] Verifying serialized route mappings...\n");
    fflush(stdout);

    bgp_sdk_route_t reloaded_route;
    memset(&reloaded_route, 0, sizeof(bgp_sdk_route_t));

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);
    size_t read_bytes = fread(&reloaded_route, sizeof(bgp_sdk_route_t), 1, file);
    assert(read_bytes == 1);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(reloaded_route.prefix_space == 0x4000);
    assert(reloaded_route.originating_as == 65001);
    assert(reloaded_route.resolved_dynamic_address == 0x51E9A4B);

    // 4. Close Coaxial Environment
    auncient_sdk_close_coaxial(&env);

    printf("   ✓ SDK BGP peering demo route verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SDK BGP NETWORKING DEMO COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
