#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

int main() {
    printf("=== TSFi Compound Kernel Lease Audit (Bypassing Hyprland) ===\n");

    // 1. Open the Physical DRM Device
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        perror("[FAIL] Could not open /dev/dri/card0");
        return 1;
    }

    // 2. Define the Compound Sovereignty Set
    // Monitor 1: Connector 100, CRTC 86
    // Monitor 2: Connector 122, CRTC 90
    // Shared Resource: Plane 71
    uint32_t objects[] = { 100, 122, 86, 90, 71 };
    uint32_t lease_id = 0;

    printf("[ACTION] Requesting Compound Lease for Plane 71 + Dual Monitors...\n");
    printf("  -> Target Objects: {100, 122, 86, 90, 71}\n");

    // 3. The Direct Kernel Handshake
    int leased_fd = drmModeCreateLease(fd, objects, 5, 0, &lease_id);

    if (leased_fd >= 0) {
        printf("[SUCCESS] !!! KERNEL SOVEREIGNTY GRANTED !!!\n");
        printf("  -> Leased FD: %d\n", leased_fd);
        printf("  -> Lease ID: %u\n", lease_id);
        printf("[INFO] Plane 71 is now coupled to both CRTCs in a private manifold.\n");
        close(leased_fd);
    } else {
        printf("[FRACTURE] Kernel rejected compound lease. (Error: %m)\n");
        printf("[ANALYSIS] This confirms that the DRM Master (Hyprland) must be relinquished or challenged before the kernel will allow this coupling.\n");
    }

    close(fd);
    printf("=== Audit Concluded ===\n");
    return 0;
}
