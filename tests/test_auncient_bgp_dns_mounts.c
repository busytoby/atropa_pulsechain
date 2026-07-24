#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_mounts.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define MAX_MOUNTS     4

// BGP Dynamic DNS Mount Point
typedef struct {
    char mount_path[64];
    uint32_t resolved_address;
    bool is_active;
} bgp_mount_t;

// BGP Server State Registry
typedef struct {
    bgp_mount_t mounts[MAX_MOUNTS];
    int mount_count;
    uint32_t status_color;
    bool switch_residency_red;
} bgp_server_t;

// Dynamic BGP Mount Point Discovery
static bool discover_and_mount_dns_endpoint(bgp_server_t *server,
                                             const char *resolved_address_str,
                                             uint32_t address) {
    printf("[BGP DISCOVERY] Intercepted DNS route for address %s...\n", resolved_address_str);
    fflush(stdout);

    // Validate compliant dynamic address resolution format
    if (strncmp(resolved_address_str, "dynamic_", 8) != 0) {
        printf("   [BGP REJECT] Non-compliant address format! Discarding route.\n");
        server->status_color = COLOR_RED;
        server->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    if (server->mount_count >= MAX_MOUNTS) {
        return false;
    }

    // Build unique mount path
    bgp_mount_t *m = &server->mounts[server->mount_count++];
    snprintf(m->mount_path, sizeof(m->mount_path), "/dns/%s/device_space", resolved_address_str);
    m->resolved_address = address;
    m->is_active = true;

    server->status_color = COLOR_CYAN;
    server->switch_residency_red = false;

    printf("   [BGP ACCEPT] Exposed dynamic mount point: %s -> 0x%X\n", m->mount_path, m->resolved_address);
    fflush(stdout);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP SERVER DYNAMIC MOUNT DISCOVERY SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bgp_server_t server = {
        .mount_count = 0,
        .status_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Discover a valid dynamic DNS resolution route and publish mount point
    printf("[TEST] Discovering valid dynamic address resolution...\n");
    fflush(stdout);
    bool ok = discover_and_mount_dns_endpoint(&server, "dynamic_0x51E9A4B", 0x51E9A4B);
    assert(ok == true);
    assert(server.switch_residency_red == false);
    assert(server.mount_count == 1);
    assert(strcmp(server.mounts[0].mount_path, "/dns/dynamic_0x51E9A4B/device_space") == 0);

    // 2. Discover an invalid route (name-based) and reject mount point
    printf("[TEST] Discovering non-compliant resolution string...\n");
    fflush(stdout);
    ok = discover_and_mount_dns_endpoint(&server, "HoganContract", 0x8888AAAA);
    assert(ok == false);
    assert(server.switch_residency_red == true);
    assert(server.status_color == COLOR_RED);

    // 3. Serialize BGP mount points to DAT file
    printf("[TEST] Deploying BGP dynamic mount points to database %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(server.mounts, sizeof(bgp_mount_t), server.mount_count, file);
    assert(written == (size_t)server.mount_count);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Verifying written database records...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);

    bgp_mount_t reloaded_mounts[MAX_MOUNTS];
    size_t read_bytes = fread(reloaded_mounts, sizeof(bgp_mount_t), server.mount_count, file);
    assert(read_bytes == (size_t)server.mount_count);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(strcmp(reloaded_mounts[0].mount_path, "/dns/dynamic_0x51E9A4B/device_space") == 0);
    assert(reloaded_mounts[0].resolved_address == 0x51E9A4B);
    assert(reloaded_mounts[0].is_active == true);

    printf("   ✓ BGP dynamic mount database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP MOUNT DISCOVERY TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
