#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

typedef struct {
    uint16_t reset_vector;
    uint16_t current_pc;
    bool boot_manager_active;
    uint32_t selected_option;
    pthread_mutex_t mutex;
} BootManager;

void boot_manager_init(BootManager *bm, uint16_t reset_vector) {
    bm->reset_vector = reset_vector;
    bm->current_pc = 0x0000;
    bm->boot_manager_active = true;
    bm->selected_option = 0;
    pthread_mutex_init(&bm->mutex, NULL);
}

void boot_manager_trigger_reset(BootManager *bm) {
    pthread_mutex_lock(&bm->mutex);
    bm->boot_manager_active = true;
    bm->current_pc = bm->reset_vector;
    pthread_mutex_unlock(&bm->mutex);
}

bool boot_manager_select(BootManager *bm, uint32_t option, uint16_t *out_pc) {
    pthread_mutex_lock(&bm->mutex);
    bm->selected_option = option;
    bm->boot_manager_active = false;
    
    if (option == 1) {
        bm->current_pc = 0x8000; // Boot Production Kernel
    } else {
        bm->current_pc = 0x4000; // Boot Diagnostics Utility
    }
    *out_pc = bm->current_pc;
    pthread_mutex_unlock(&bm->mutex);
    return true;
}

// Hypervisor-level transformation of arbitrary guest memory space
bool hypervisor_transform_memory(uint8_t *guest_memory, size_t memory_size, uint32_t start_address, size_t length, uint8_t key) {
    // Prevent out-of-bounds guest memory modifications
    if (start_address + length > memory_size) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        guest_memory[start_address + i] ^= key;
    }
    
    return true;
}
