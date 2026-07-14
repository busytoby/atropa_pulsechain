#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// PDP-11 Style MMIO Register Definitions (UNIBUS address mapped registers)
#define MMIO_BASE_ADDR   0x7FFF0000
#define CSR_REG_OFFSET   0x00       // Control Status Register
#define DATA_REG_OFFSET  0x04       // Data buffer port
#define PLL_DEV_OFFSET   0x08       // Phase-locked Loop deviation offset

// Mock MCP Peer Structure
typedef struct {
    char service_name[32];
    uint8_t buffer[256];
    bool is_active;
    uint32_t phase_lock_status;
} McpPeerService;

// Dispatch table structure for the Central ZMM Dispatcher
typedef struct {
    uintptr_t base_address;
    size_t page_size;
    McpPeerService *mounted_peer;
    bool fast_path_enabled;
} ZmmDispatchEntry;

#define MAX_MOUNTS 8
ZmmDispatchEntry g_dispatch_table[MAX_MOUNTS];

// Initialize dispatch system
void init_dispatch_system(void) {
    memset(g_dispatch_table, 0, sizeof(g_dispatch_table));
}

// Register a PDP-11 style mount
bool register_pdp11_mount(uintptr_t base_addr, size_t size, McpPeerService *peer) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (g_dispatch_table[i].base_address == 0) {
            g_dispatch_table[i].base_address = base_addr;
            g_dispatch_table[i].page_size = size;
            g_dispatch_table[i].mounted_peer = peer;
            g_dispatch_table[i].fast_path_enabled = true;
            return true;
        }
    }
    return false;
}

// Deregister a PDP-11 style mount
bool deregister_pdp11_mount(uintptr_t base_addr) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (g_dispatch_table[i].base_address == base_addr) {
            g_dispatch_table[i].base_address = 0;
            g_dispatch_table[i].page_size = 0;
            g_dispatch_table[i].mounted_peer = NULL;
            g_dispatch_table[i].fast_path_enabled = false;
            return true;
        }
    }
    return false;
}

// MMIO Write operation handled by the dispatcher JIT loopback
bool dispatcher_write(uintptr_t addr, uint32_t val, const char **out_err) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        ZmmDispatchEntry *entry = &g_dispatch_table[i];
        if (entry->base_address != 0 && addr >= entry->base_address && addr < entry->base_address + entry->page_size) {
            if (!entry->fast_path_enabled || !entry->mounted_peer) {
                *out_err = "REVERT: MOUNTED_PEER_NOT_AVAILABLE";
                return false;
            }
            
            uintptr_t offset = addr - entry->base_address;
            if (offset == CSR_REG_OFFSET) {
                entry->mounted_peer->phase_lock_status = val;
            } else if (offset == DATA_REG_OFFSET) {
                entry->mounted_peer->buffer[0] = (uint8_t)(val & 0xFF);
                entry->mounted_peer->buffer[1] = (uint8_t)((val >> 8) & 0xFF);
            } else if (offset == PLL_DEV_OFFSET) {
                // Modulate phase deviation values on dynamic WinchesterMQ loops
                entry->mounted_peer->buffer[2] = (uint8_t)(val & 0xFF);
            }
            return true;
        }
    }
    *out_err = "REVERT: ILLEGAL_MMIO_ADDRESS_ACCESS";
    return false;
}

// MMIO Read operation handled by the dispatcher JIT loopback
bool dispatcher_read(uintptr_t addr, uint32_t *out_val, const char **out_err) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        ZmmDispatchEntry *entry = &g_dispatch_table[i];
        if (entry->base_address != 0 && addr >= entry->base_address && addr < entry->base_address + entry->page_size) {
            if (!entry->fast_path_enabled || !entry->mounted_peer) {
                *out_err = "REVERT: MOUNTED_PEER_NOT_AVAILABLE";
                return false;
            }
            
            uintptr_t offset = addr - entry->base_address;
            if (offset == CSR_REG_OFFSET) {
                *out_val = entry->mounted_peer->phase_lock_status;
            } else if (offset == DATA_REG_OFFSET) {
                *out_val = (uint32_t)entry->mounted_peer->buffer[0] | ((uint32_t)entry->mounted_peer->buffer[1] << 8);
            } else if (offset == PLL_DEV_OFFSET) {
                *out_val = (uint32_t)entry->mounted_peer->buffer[2];
            }
            return true;
        }
    }
    *out_err = "REVERT: ILLEGAL_MMIO_ADDRESS_ACCESS";
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: PDP-11 STYLE MMIO MOUNT & JIT TESTS\n");
    printf("=============================================================\n");

    init_dispatch_system();

    // Create custom service peer
    McpPeerService keyboard_service;
    memset(&keyboard_service, 0, sizeof(keyboard_service));
    strcpy(keyboard_service.service_name, "WinchesterMQ_Keyboard");
    keyboard_service.is_active = true;

    const char *err = NULL;
    uint32_t read_val = 0;

    // 1. Unregistered access should fail
    printf("1. Accessing unregistered MMIO address space...\n");
    bool ok = dispatcher_write(MMIO_BASE_ADDR + DATA_REG_OFFSET, 0xAA55, &err);
    assert(!ok);
    assert(strcmp(err, "REVERT: ILLEGAL_MMIO_ADDRESS_ACCESS") == 0);
    printf("   ✓ Correctly blocked unregistered access.\n\n");

    // 2. Register PDP-11 Style mount
    printf("2. Registering peer keyboard service on address space 0x%X...\n", MMIO_BASE_ADDR);
    bool mount_ok = register_pdp11_mount(MMIO_BASE_ADDR, 256, &keyboard_service);
    assert(mount_ok);
    printf("   ✓ Mount registered successfully.\n\n");

    // 3. Write data to mounted register
    printf("3. Writing data 0x55AA to data register...\n");
    ok = dispatcher_write(MMIO_BASE_ADDR + DATA_REG_OFFSET, 0x55AA, &err);
    assert(ok);
    assert(keyboard_service.buffer[0] == 0xAA);
    assert(keyboard_service.buffer[1] == 0x55);
    printf("   ✓ Written bytes matching PDP-11 byte order layout.\n\n");

    // 4. Read back the register status
    printf("4. Reading back register data via JIT-wired loopback...\n");
    ok = dispatcher_read(MMIO_BASE_ADDR + DATA_REG_OFFSET, &read_val, &err);
    assert(ok);
    assert(read_val == 0x55AA);
    printf("   ✓ Read value matching input: 0x%X.\n\n", read_val);

    // 5. Deregister the mount and ensure it is clean
    printf("5. Deregistering mount address 0x%X...\n", MMIO_BASE_ADDR);
    bool unmount_ok = deregister_pdp11_mount(MMIO_BASE_ADDR);
    assert(unmount_ok);
    
    ok = dispatcher_read(MMIO_BASE_ADDR + DATA_REG_OFFSET, &read_val, &err);
    assert(!ok);
    assert(strcmp(err, "REVERT: ILLEGAL_MMIO_ADDRESS_ACCESS") == 0);
    printf("   ✓ Mount successfully detached and unmapped.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT PDP-11 STYLE MMIO MOUNT TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
