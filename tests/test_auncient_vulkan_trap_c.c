#include "../firmware/tsfi_lib/tsfi_vulkan_trap.c"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auncient Vulkan Frame-Buffer Trapping C Verification Suite\n");
    printf("=============================================================\n");

    VulkanFrameTrap trap;
    vulkan_trap_init(&trap);

    // 1. Initial State
    assert(trap.frame_index == 0);
    assert(trap.is_locked == false);
    printf("   ✓ Frame trap system initialized.\n");

    // 2. Generate and write mock RGBA pixel data
    printf("2. Writing mock 800x600 RGBA frame...\n");
    uint8_t *mock_pixels = (uint8_t*)malloc(FRAME_BUFFER_SIZE);
    memset(mock_pixels, 0x5A, FRAME_BUFFER_SIZE); // Fill frame with 0x5A

    bool trap_ok = vulkan_trap_frame(&trap, mock_pixels, FRAME_BUFFER_SIZE);
    assert(trap_ok == true);
    assert(trap.frame_index == 1);
    printf("   ✓ Frame captured by hypervisor. Frame index: %u\n", trap.frame_index);

    // 3. Lock and read frame from guest VM side
    printf("3. Locking frame buffer to verify guest memory mapping...\n");
    const uint8_t *guest_pixels = vulkan_trap_lock_guest_frame(&trap);
    assert(trap.is_locked == true);
    
    // Verify first and last bytes of guest VRAM match mock source
    assert(guest_pixels[0] == 0x5A);
    assert(guest_pixels[FRAME_BUFFER_SIZE - 1] == 0x5A);
    printf("   ✓ Guest read verified: Byte 0: 0x%02X, Byte %d: 0x%02X\n", 
           guest_pixels[0], FRAME_BUFFER_SIZE - 1, guest_pixels[FRAME_BUFFER_SIZE - 1]);

    // 4. Test frame drop when guest locks buffer
    printf("4. Testing frame-drop on concurrent write attempts...\n");
    bool dropped = vulkan_trap_frame(&trap, mock_pixels, FRAME_BUFFER_SIZE);
    assert(dropped == false); // Should drop frame since guest holds lock
    assert(trap.frame_index == 1); // Frame index remains unchanged
    printf("   ✓ Concurrency lock verified (frame successfully dropped).\n");

    // 5. Unlock and clean up
    vulkan_trap_unlock_guest_frame(&trap);
    assert(trap.is_locked == false);
    
    vulkan_trap_destroy(&trap);
    free(mock_pixels);

    printf("\n=============================================================\n");
    printf("AUNCIENT VULKAN FRAME TRAPPING TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
