#include "../firmware/tsfi_lib/tsfi_mci_routing.c"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auncient Multi-LUN Routing & Token Gate C Verification Suite\n");
    printf("=============================================================\n");

    MCIRouter router;
    mci_init(&router, 0x485301a0);

    // 1. Blocked when unauthorized
    const char *res1 = mci_route_write(&router, 42);
    assert(strcmp(res1, "BLOCKED") == 0);
    printf("   ✓ Blocked unauthorized writes correctly.\n");

    // 2. Authorize
    uint32_t valid_token = (0x485301a0 ^ 0x2600) & 0xFFFF;
    bool auth_success = mci_authorize(&router, valid_token);
    assert(auth_success == true);
    printf("   ✓ Authorized dynamic MCI challenge token.\n");

    // 3. Route LUN 0
    const char *resL0 = mci_route_write(&router, 100);
    assert(strcmp(resL0, "AVX_THUNK") == 0);
    printf("   ✓ LUN 0 routed to AVX thunk correctly.\n");

    // 4. Route LUN 1
    mci_set_lun(&router, 1);
    const char *resL1 = mci_route_write(&router, 120);
    assert(strcmp(resL1, "ALSA_THUNK") == 0);
    printf("   ✓ LUN 1 routed to ALSA audio thunk correctly.\n");

    // 5. Route LUN 2
    mci_set_lun(&router, 2);
    const char *resL2 = mci_route_write(&router, 255);
    assert(strcmp(resL2, "VULKAN_THUNK") == 0);
    printf("   ✓ LUN 2 routed to Vulkan display thunk correctly.\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT MULTI-LUN ROUTING C IMPLEMENTATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
