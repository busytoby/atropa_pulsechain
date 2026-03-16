#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "tsfi_plugin.h"
#include "tsfi_logic.h"
#include "lau_memory.h"

// --- Custom System Definition ---
DEFINE_MAPPED_STRUCT(CustomWaveSystem, 
    int operation_mode; 
    char *session_token;
    int *data_BASE;
    size_t BASE_size;
)

static CustomWaveSystem *my_system = NULL;

// --- Helper: Context Recovery ---
// Recovers the CustomWaveSystem pointer from the address of the version field.
// This works because we know 'ver' points to h->version.
CustomWaveSystem* get_context_from_version(int *ver) {
    // 1. Calculate offset of 'version' inside LauWiredHeader
    size_t offset_version = offsetof(LauWiredHeader, version);
    
    // 2. Calculate start of Header
    LauSystemHeader *h = (LauSystemHeader *)((char*)ver - offset_version);
    
    // 3. Calculate start of Payload (CustomWaveSystem)
    // Payload is immediately after Header
    return (CustomWaveSystem *)((char*)h + offsetof(LauWiredHeader, payload));
}

// --- Inner Logic ---

void custom_epoch(int *ver) {
    *ver += 1;
    
    // Demonstrate Context Recovery
    CustomWaveSystem *cws = get_context_from_version(ver);
    
    if (cws && cws->data_BASE && cws->BASE_size > 0) {
        // Autonomous Logic: Rotate BASE values based on version
        for (size_t i = 0; i < cws->BASE_size; i++) {
            cws->data_BASE[i] += (*ver % 3); 
        }
        printf("  [CUSTOM] Epoch Autonomy: Updated %zu BASE cells.\n", cws->BASE_size);
    }
}

void custom_state(bool *ftw) {
    *ftw = true; 
}

void custom_directive(int *cnt, char *dir) {
    (*cnt)++;
    // printf("  [CUSTOM] Inner Directive: %s\n", dir); // Reduced spam
    
    if (my_system) {
        if (strncmp(dir, "ALLOC_BASE ", 11) == 0) {
            int size = 0;
            if (sscanf(dir + 11, "%d", &size) == 1 && size > 0) {
                if (my_system->data_BASE) lau_free(my_system->data_BASE);
                my_system->BASE_size = size;
                my_system->data_BASE = (int*)lau_malloc(size * sizeof(int));
                // Initialize
                for(int i=0; i<size; i++) my_system->data_BASE[i] = i * 10;
                printf("  [CUSTOM] Allocated BASE: %d ints\n", size);
            }
        }
        else if (strcmp(dir, "VERIFY_BASE") == 0) {
            if (my_system->data_BASE) {
                printf("  [CUSTOM] BASE Dump [0..%zu]: ", my_system->BASE_size > 5 ? 5 : my_system->BASE_size);
                for (size_t i=0; i < (my_system->BASE_size > 5 ? 5 : my_system->BASE_size); i++) {
                    printf("%d ", my_system->data_BASE[i]);
                }
                printf("...\n");
            } else {
                printf("  [CUSTOM] BASE is NULL\n");
            }
        }
    }
}

void custom_scramble(void *ws_ptr) {
    CustomWaveSystem *cws = (CustomWaveSystem*)ws_ptr;
    printf("  [CUSTOM] Scramble! Token & BASE.\n");
    if (cws->session_token) {
        lau_mem_scramble(cws->session_token, strlen(cws->session_token), LAU_SCRAMBLE_MODE_RANDOM);
    }
    if (cws->data_BASE && cws->BASE_size > 0) {
        // Securely scramble the BASE data
        lau_mem_scramble(cws->data_BASE, cws->BASE_size * sizeof(int), 0x00); // Zero it out for safety
        printf("  [CUSTOM] BASE zeroed.\n");
    }
}

void custom_provenance(void *ws_ptr) {
    CustomWaveSystem *cws = (CustomWaveSystem*)ws_ptr;
    printf("  [CUSTOM] Provenance Mode: %d | BASE Size: %zu\n", cws->operation_mode, cws->BASE_size);
}

// --- Constructor ---
CustomWaveSystem* create_custom_system(void) {
    CustomWaveSystem *cws = (CustomWaveSystem *)lau_malloc_wired(sizeof(CustomWaveSystem));
    if (!cws) return NULL;

    LauSystemHeader *h = (LauSystemHeader *)((char *)cws - offsetof(LauWiredHeader, payload));
    h->resonance_as_status = lau_strdup("CUSTOM_INIT");
    
    lau_wire_system((WaveSystem*)cws, h, tsfi_get_default_logic());
    
    cws->operation_mode = 1;
    cws->session_token = lau_strdup("SESSION_ALPHA_99");
    cws->data_BASE = NULL;
    cws->BASE_size = 0;
    
    return cws;
}

// --- Master Logic Implementation ---

void master_logic_epoch(int *ver) { 
    *ver = 3000; 
    if (my_system) {
        printf("[PLUGIN] Stepping Custom System...\n");
        my_system->step_safety_epoch();
    }
}

void master_logic_state(bool *ftw) { 
    *ftw = true; 
    if (my_system) my_system->step_safety_state();
}

void master_logic_directive(int *cnt, char *dir) { 
    if (dir) {
        (*cnt)++; 
        printf("[PLUGIN] Directive '%s' intercepted!\n", dir);
        
        if (strcmp(dir, "INIT_CUSTOM") == 0) {
            if (!my_system) {
                my_system = create_custom_system();
                if (my_system) {
                    TSFiLogicTable logic = {
                        .logic_epoch = custom_epoch,
                        .logic_state = custom_state,
                        .logic_directive = custom_directive,
                        .logic_scramble = custom_scramble,
                        .logic_provenance = custom_provenance
                    };
                    lau_update_logic((WaveSystem*)my_system, &logic);
                    
                    if (*my_system->resonance_as_status) lau_free(*my_system->resonance_as_status);
                    *my_system->resonance_as_status = lau_strdup("CUSTOM_ACTIVE");
                    
                    printf("[PLUGIN] Custom System Initialized.\n");
                }
            } else {
                printf("[PLUGIN] Custom System already exists.\n");
            }
        }
        else if (strncmp(dir, "CMD_CUSTOM ", 11) == 0) {
            if (my_system) {
                my_system->step_executor_directive(dir + 11);
            }
        }
        else if (strcmp(dir, "DESTROY_CUSTOM") == 0) {
            if (my_system) {
                if (my_system->session_token) lau_free(my_system->session_token);
                if (my_system->data_BASE) lau_free(my_system->data_BASE);
                lau_free(my_system);
                my_system = NULL;
                printf("[PLUGIN] Custom System Destroyed.\n");
            }
        }
        else {
            // Forward unknown directives to the host's chained logic
            // This allows tsfi_vulkan to receive CLOSE_WINDOW even if we are the primary logic.
            chained_logic_directive(cnt, dir);
        }
    }
}

void master_logic_scramble(void *ws_ptr) {
    (void)ws_ptr;
    printf("[CUSTOM] Scramble! Token & BASE initialized.\n");
}

void master_logic_provenance(void *ws_ptr) {
    WaveSystem *ws = (WaveSystem*)ws_ptr;
    printf("[PLUGIN] Provenance check intercepted for System ID: %d\n", ws->system_id);
    if (my_system) {
        my_system->provenance();
    }
}

