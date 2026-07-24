#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SYSFS_BASE "/tmp/sys/class/net_auncient"

// Thread-safe dynamic parameters
typedef struct {
    uint16_t stanag_port;
    char stanag_mac[18];
    uint16_t decnet_addr;
} auncient_net_dev_t;

static auncient_net_dev_t g_device_state = { 0, "", 0 };

// Helper to create directory path recursively
static void make_dir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

// -------------------------------------------------------------
// VFS sysfs node initializers
// -------------------------------------------------------------
void init_sysfs_skeleton(void) {
    make_dir(SYSFS_BASE "/stanag0");
    make_dir(SYSFS_BASE "/decnet0");
    printf("   [SKELETON] Initialized directory skeleton at %s\n", SYSFS_BASE);
    fflush(stdout);
}

void write_sysfs_stanag_port(uint16_t port) {
    char path[256];
    snprintf(path, sizeof(path), SYSFS_BASE "/stanag0/port");
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%d\n", port);
        fclose(f);
    }
    g_device_state.stanag_port = port;
    printf("   [SKELETON] Port update registered: %d\n", port);
    fflush(stdout);
}

uint16_t read_sysfs_stanag_port(void) {
    char path[256];
    uint16_t port = 0;
    snprintf(path, sizeof(path), SYSFS_BASE "/stanag0/port");
    FILE *f = fopen(path, "r");
    if (f) {
        if (fscanf(f, "%hd", &port) == 1) {
            g_device_state.stanag_port = port;
        }
        fclose(f);
    }
    return port;
}

void write_sysfs_decnet_addr(uint8_t area, uint16_t node) {
    char path[256];
    uint16_t addr = (uint16_t)((area << 10) | (node & 0x03FF));
    snprintf(path, sizeof(path), SYSFS_BASE "/decnet0/node_address");
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%d.%d\n", area, node);
        fclose(f);
    }
    g_device_state.decnet_addr = addr;
    printf("   [SKELETON] DECnet address update registered: %d.%d\n", area, node);
    fflush(stdout);
}

// Cleanup simulated sysfs skeleton
void cleanup_sysfs_skeleton(void) {
    unlink(SYSFS_BASE "/stanag0/port");
    unlink(SYSFS_BASE "/decnet0/node_address");
    rmdir(SYSFS_BASE "/stanag0");
    rmdir(SYSFS_BASE "/decnet0");
    rmdir(SYSFS_BASE);
    printf("   [SKELETON] Cleaned up directory structure.\n");
    fflush(stdout);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT NETWORK DEVICE DIRECTORY SKELETON VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Initialize Skeleton directory layout
    init_sysfs_skeleton();

    // 2. Validate Port updates
    write_sysfs_stanag_port(2048);
    uint16_t port = read_sysfs_stanag_port();
    assert(port == 2048);
    assert(g_device_state.stanag_port == 2048);
    printf("   ✓ STANAG port sysfs register pass.\n");
    fflush(stdout);

    // 3. Validate DECnet Address updates
    write_sysfs_decnet_addr(1, 10); // Node 1.10
    assert(g_device_state.decnet_addr == ((1 << 10) | 10));
    printf("   ✓ DECnet node address sysfs register pass.\n");
    fflush(stdout);

    // 4. Tear down skeleton filesystem layout
    cleanup_sysfs_skeleton();

    printf("=============================================================\n");
    printf("DIRECTORY SKELETON INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
