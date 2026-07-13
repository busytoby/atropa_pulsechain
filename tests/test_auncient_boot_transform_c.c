#include "../firmware/tsfi_lib/tsfi_boot_manager.c"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auncient Boot Manager & Memory Transformer C Verification Suite\n");
    printf("=============================================================\n");

    BootManager bm;
    boot_manager_init(&bm, 0xFF5C);

    // 1. Reset
    boot_manager_trigger_reset(&bm);
    assert(bm.current_pc == 0xFF5C);
    assert(bm.boot_manager_active == true);
    printf("   ✓ Warm-reset boot PC intercepted correctly.\n");

    // 2. Select option 1
    uint16_t out_pc = 0;
    boot_manager_select(&bm, 1, &out_pc);
    assert(out_pc == 0x8000);
    assert(bm.boot_manager_active == false);
    printf("   ✓ Boot selection routed to production vector correctly.\n");

    // 3. Memory transformation test
    uint8_t guest_ram[4096];
    memset(guest_ram, 0, sizeof(guest_ram));
    
    // Write target test payload
    const char *payload = "Auncient";
    size_t len = strlen(payload);
    uint32_t offset = 0x600;
    uint8_t key = 0xAA;

    for (size_t i = 0; i < len; i++) {
        guest_ram[offset + i] = (uint8_t)payload[i] ^ key;
    }

    // Run hypervisor transformation (decrypt)
    bool success = hypervisor_transform_memory(guest_ram, sizeof(guest_ram), offset, len, key);
    assert(success == true);

    // Assert decrypted matches original payload
    assert(memcmp(&guest_ram[offset], payload, len) == 0);
    printf("   ✓ Hypervisor-level memory decryption verified successfully.\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT BOOT MANAGER & MEMORY TRANSFORM C TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
